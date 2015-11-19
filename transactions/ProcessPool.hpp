#pragma once
#include "SafeSet.hpp"
#include "ctpl_stl.h"
#include "SerializationSupport.hpp"
#include <unistd.h>
#include <signal.h>
#include "compile-time-tuple.hpp"

template<typename Ret, typename... Arg>
class ProcessPool{

	class Child{
		pid_t name;
		int parent_to_child[2];
		int child_to_parent[2];
		std::vector<std::function<Ret (Arg...)> > &behaviors;

		struct ReadError{};

		auto _populate_arg(const ct::tuple<> &a){
			return a;
		}
		
		template<typename A, typename... Elts>
		auto _populate_arg(const ct::tuple<std::shared_ptr<A>, std::shared_ptr<Elts>... > &arg){
			int size = -1;
			assert(read(parent_to_child[0],&size,sizeof(size)) > 0);
			std::vector<char> recv (size);
			if (read(parent_to_child[0],recv.data(),size) <= 0)
				throw ReadError{};
			else{
				return ct::cons(from_bytes<A>(recv.data()),populate_arg(arg.rest));
			}
		};
		
		template<typename CT>
		auto populate_arg(const CT &ct){
			return _populate_arg(ct).to_std_tuple();
		}

		auto behaviorOnPointers(int command, std::shared_ptr<Args>... args){
			return behaviors.at(command)((*args)...);
		}
		
		void childSpin(){
			int command;
			try{
				while (read(parent_to_child[0],&command,sizeof(command)) > 0){
					auto args = populate_arg(ct::tuple<std::shared_ptr<Arg>...>{});
					auto ret = callFunc(behaviorOnPointers,
										std::tuple_cat(std::make_tuple(command),args));
					int size = bytes_size(ret);
					std::vector<char> bytes(size);
					assert(bytes.size() == to_bytes(ret,bytes.data()));
					write(child_to_parent[1],&size,sizeof(size));
					write(child_to_parent[1],bytes.data(),size);
				}
			}
			catch (const ReadError&) {/* abort thread*/}

		}

		void _command(){}

		template<typename Arg1, typename... Arg2>
		void _command(const Arg1 & arg, const Arg2 & ... rest){
			int size = bytes_size(arg);
			std::vector<char> bytes(size);
			assert(bytes.size() == to_bytes(arg,bytes.data()));
			write(parent_to_child[1],&size,sizeof(size));
			write(parent_to_child[1],bytes.data(),size);
		}

		void command(int com, const Arg & ... arg){
			write(parent_to_child[1],&com,sizeof(com));
			_command(arg);
		}
	public:

		bool operator==(const Child &c) const {
			assert(name != 0);
			assert(c.name != 0);
			return name == c.name;
		}
		
		Child(decltype(behaviors) b):behaviors(b){
			pipe(parent_to_child);
			pipe(child_to_parent);
			std::cout << "forking new child!" << std::endl;
			name = fork();
		}
		Child(const Child&) = delete;
		virtual ~Child(){
			close(parent_to_child[1]);
			close(child_to_parent[0]);
			close(parent_to_child[0]);
			close(child_to_parent[1]);
		}
		friend class ProcessPool;
	};
	
	const int limit;
	ctpl::thread_pool tp;
	SafeSet<Child> children;
	SafeSet<Child*> ready;
	std::vector<std::function<Ret (Arg...)> > behaviors;

	std::unique_ptr<Ret> waitOnChild(Child &c){
		std::cout << "waiting on child" << std::endl;
		AtScopeEnd ase{[](){std::cout << "child wait thread done" << std::endl;}};
		int size;
		if(read(c.child_to_parent[0],&size,sizeof(size)) > 0){
			std::vector<char> v(size);
			read(c.child_to_parent[0],v.data(),size);
			ready.add(&c);
			return from_bytes<Ret>(v.data());
		}
		else {
			kill(c.name, SIGKILL);
			children.remove(c);
			return nullptr;
		}
	}
public:
	
	ProcessPool (std::vector<std::function<Ret (Arg...)> > beh, int limit = 200):limit(limit),tp(limit),behaviors(beh)
		{}
	
	auto launch(int command, const Arg & ... arg){
		if (children.size() < limit){
			auto &child = children.emplace(behaviors);
			if (child.name == 0) {
				child.childSpin();
				exit(0);
			}
			else ready.add(&child);
		}
		while (ready.empty()) {}
		if (!ready.empty()){
			Child &cand = *ready.pop();
			cand.command(command,arg...);
			return tp.push([&](int){return waitOnChild(cand);});
		}
		else assert(false && "um you screwed up threads really bad dude");
	}

	virtual ~ProcessPool(){
		typename decltype(children)::lock l{children.m};
		discard(l);
		for (auto &c : children.impl){
			if (c.name != 0)
				kill(c.name,SIGKILL);
		}

	}

};
