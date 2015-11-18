#pragma once
#include "SafeSet.hpp"
#include "ctpl_stl.h"
#include "SerializationSupport.hpp"
#include <unistd.h>
#include <signal.h>

template<typename Ret, typename Arg>
class ProcessPool{

	class Child{
		pid_t name;
		int parent_to_child[2];
		int child_to_parent[2];
		std::vector<std::function<Ret (Arg)> > &behaviors;
		void childSpin(){
			int command;
			Arg arg;
			while ((read(parent_to_child[0],&command,sizeof(command)) > 0)
				   && (read(parent_to_child[0],&arg,sizeof(arg)) > 0)){
				auto ret = behaviors.at(command)(arg);
				int size = bytes_size(ret);
				std::vector<char> bytes(size);
				assert(bytes.size() == to_bytes(ret,bytes.data()));
				write(child_to_parent[1],&size,sizeof(size));
				write(child_to_parent[1],bytes.data(),size);
			}
		}

		void command(int com, Arg arg){
			write(parent_to_child[1],&com,sizeof(com));
			write(parent_to_child[1],&arg,sizeof(arg));
		}
	public:
		
		bool operator<(const Child &c) const {
			return (name < c.name)
				&& (parent_to_child[0] < c.parent_to_child[0])
				&& (parent_to_child[1] < c.parent_to_child[1])
				&& (child_to_parent[0] < c.child_to_parent[0])
				&& (child_to_parent[1] < c.child_to_parent[1]);
		}
		
		Child(decltype(behaviors) b):behaviors(b){
			pipe(parent_to_child);
			pipe(child_to_parent);
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
	std::vector<std::function<Ret (Arg)> > behaviors;

	std::unique_ptr<Ret> waitOnChild(Child &c){
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

	ProcessPool (std::vector<std::function<Ret (Arg)> > beh, int limit = 201):limit(limit),tp(limit),behaviors(beh)
		{}
	
	auto launch(int command, Arg arg){
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
			cand.command(command,arg);
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
