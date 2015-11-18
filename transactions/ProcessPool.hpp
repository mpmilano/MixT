#pragma once
#include "SafeSet.hpp"
#include "ctpl_stl.h"
#include "SerializationSupport.hpp"

template<typename Ret, typename Arg>
class ProcessPool{

	class Child{
		pid_t name;
		int parent_to_child[2];
		int child_to_parent[2];
		std::vector<std::function<Ret (Arg)> > &behaviors;
		Child(decltype(behaviors) b):behaviors(b){
			pipe(parent_to_child);
			pipe(child_to_parent);
			pid = fork();
		}
		void childSpin(){
			int command;
			Arg arg;
			while ((read(parent_to_child[0],&command,sizeof(command)) > 0)
				   && (read(parent_to_child[0],&arg,sizeof(arg)) > 0)){
				auto ret = behaviors.at(command)(arg);
				int size = bytes_size(ret);
				std::vector<char> bytes{size};
				assert(bytes.size() == to_bytes(ret,bytes.data()));
				write(child_to_parent[1],size,sizeof(size));
				write(child_to_parent[1],bytes.data(),size);
			}
		}

		void command(int com, Arg arg){
			write(parent_to_child[1],com,sizeof(com));
			write(parent_to_child[1],arg,sizeof(arg));
		}
		friend class ProcessPool;
	public:
		Child(const Child&) = delete;
		virtual ~Child(){
			close(parent_to_child[1]);
			close(child_to_parent[0]);
			close(parent_to_child[0]);
			close(child_to_parent[1]);
		}
	};
	
	const int limit;
	ctpl::thread_pool tp(limit);
	SafeSet<Child> children;
	SafeSet<Child*> ready;
	std::vector<std::function<Ret (Arg)> > behaviors;

	std::unique_ptr<Ret> waitOnChild(Child &c){
		int size;
		if(read(c.child_to_parent[0],&size,sizeof(size)) > 0){
			std::vector<char> v(size);
			read(c.child_to_parent[0],v.data(),size);
			ready.add(&c);
			return from_bytes<ret>(v.data());
		}
		else {
			kill(c.name, SIGKILL);
			children.remove(c);
			return nullptr;
		}
	}

	ProcessPool (std::vector<std::function<Ret (Arg)> > beh, int limit = 201):limit(limit),behaviors(beh)
		{}
	
	auto launch(int command, Arg arg){
		if (pids.size() < limit){
			
			auto &child = children.emplace(behaviors);
			if (child.pid == 0) {
				child.childSpin();
				exit(0);
			}
			else ready.add(&child);
		}
		do {
			if (!ready.empty()){
				Child &cand = ready.pop();
				cand.command(command,arg);
				return tp.push([&](){waitOnChild(cand)};);
			}
		} while (ready.empty());
	}

	~ProcessPool(){
		for (auto &c : children){
			if (c.name != 0)
				kill(c.name,SIGKILL);
		}
		for (auto &t : waiters){
			t.get();
		}
	}

};
