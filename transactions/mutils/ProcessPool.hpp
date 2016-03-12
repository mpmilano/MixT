#pragma once
#include "SafeSet.hpp"
#include "ctpl_stl.h"
#include "SerializationSupport.hpp"
#include <unistd.h>
#include <signal.h>
#include <exception>
#include "compile-time-tuple.hpp"
#include "TaskPool.hpp"


//ProcessPool is for times when your libraries just really like static memory, but remember to only call async_signal_safe things!

namespace mutils{

	template<typename Mem, typename Ret, typename... Arg>
	class ProcessPool_impl : public TaskPool_impl<ProcessPool_impl<Mem,Ret,Arg...>,Mem,Ret,Arg...>{

		class Child{
			pid_t name;
			int parent_to_child[2];
			int child_to_parent[2];
			std::vector<std::function<Ret (std::unique_ptr<Mem>&, int, Arg...)> > &behaviors;
			const std::function<Ret (std::exception_ptr)> &onException;

			struct ReadError{};

			auto _populate_arg(const ct::tuple<> &a){
				return a;
			}
		
			template<typename A, typename... Elts>
			auto _populate_arg(const ct::tuple<std::shared_ptr<A>, std::shared_ptr<Elts>... > &arg){
				int size = -1;
				auto readres = read(parent_to_child[0],&size,sizeof(size));
				assert(readres > 0);
				std::vector<char> recv (size);
				if (read(parent_to_child[0],recv.data(),size) <= 0)
					throw ReadError{};
				else{
					return ct::cons(std::shared_ptr<A>(from_bytes<A>(nullptr,
																	 recv.data()).release()),_populate_arg(arg.rest));
				}
			};
		
			template<typename CT>
			auto populate_arg(const CT &ct){
				return _populate_arg(ct).to_std_tuple();
			}

			static auto behaviorOnPointers(std::vector<std::function<Ret (std::unique_ptr<Mem>&, int, Arg...)> > const * const behaviors, std::unique_ptr<Mem>* remember, int command, int name, std::shared_ptr<Arg>... args){
				return behaviors->at(command)(*remember,name,(*args)...);
			}

			void writeBackResult(const Ret &ret){
				int size = bytes_size(ret);
				std::vector<char> bytes(size);
				auto tbs = to_bytes(ret,bytes.data());
				assert(bytes.size() == tbs);
				write(child_to_parent[1],&size,sizeof(size));
				write(child_to_parent[1],bytes.data(),size);
			}

			std::unique_ptr<Mem> remember;
			
			void childSpin(){
				int command;
				int name;
				try{
					while (read(parent_to_child[0],&command,sizeof(command)) > 0){
						auto readres = read(parent_to_child[0],&name,sizeof(name));
						assert(readres > 0);
						auto args = populate_arg(ct::tuple<std::shared_ptr<Arg>...>{});
						try{
							auto ret = callFunc(behaviorOnPointers,
												std::tuple_cat(std::make_tuple(&behaviors,&remember,command,name),args));
							writeBackResult(ret);
						}
						catch(...){
							writeBackResult(onException(std::current_exception()));
						}
					}
				}
				catch (const ReadError&) {/* abort thread*/}

			}

			void _command(){}

			template<typename Arg1, typename... Arg2>
			void _command(const Arg1 & arg, const Arg2 & ... rest){
				int size = bytes_size(arg);
				std::vector<char> bytes(size);
				auto tbs = to_bytes(arg,bytes.data());
				assert(bytes.size() == tbs);
				write(parent_to_child[1],&size,sizeof(size));
				write(parent_to_child[1],bytes.data(),size);
				_command(rest...);
			}

			void command(int com, int name, const Arg & ... arg){
				write(parent_to_child[1],&com,sizeof(com));
				write(parent_to_child[1],&name,sizeof(name));
				_command(arg...);
			}
		public:

			bool operator==(const Child &c) const {
				assert(name != 0);
				assert(c.name != 0);
				return name == c.name;
			}
		
			Child(decltype(behaviors) b, decltype(onException) e)
				:behaviors(b),onException(e){
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
				if (name != 0){
					kill(name, SIGKILL);
				}
			}
			friend class ProcessPool_impl;
		};
	

		SafeSet<Child> children;
		SafeSet<Child*> ready;

		std::unique_ptr<Ret> waitOnChild(Child &c){
			int size;
			if(read(c.child_to_parent[0],&size,sizeof(size)) > 0){
				std::vector<char> v(size);
				read(c.child_to_parent[0],v.data(),size);
				ready.add(&c);
				return from_bytes<Ret>(nullptr,v.data());
			}
			else {
				std::cerr << "child process " << c.name << " exited abnormally" << std::endl;
				kill(c.name, SIGKILL);
				children.remove(c);
				return nullptr;
			}
		}
	
	public:
	
		ProcessPool_impl (std::shared_ptr<ProcessPool_impl> &pp,
						  std::vector<std::function<Ret (std::unique_ptr<Mem>&, int, Arg...)> > beh,
						  int limit,
						  std::function<Ret (std::exception_ptr)> onException
			):TaskPool_impl<ProcessPool_impl<Mem,Ret,Arg...>,Mem,Ret,Arg...>(pp,beh,limit,onException)
			{
				while (children.size() < limit){
					auto &child = children.emplace(this->behaviors,this->onException);
					if (child.name == 0) {
						child.childSpin();
						exit(0);
					}
					else {
						close(child.child_to_parent[1]);
						close(child.parent_to_child[0]);
						ready.add(&child);
					}
				}

				//this->tp.reset(new ctpl::thread_pool{limit});
			}
		
		std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg){
			auto this_sp = this->this_sp;
			assert(this_sp.get() == this);
			return this->tp->push([this_sp,command,arg...](int) -> std::unique_ptr<Ret>{
					//we should only be scheduled when there's something to grab,
					//so this should never spin-lock.
					while (this_sp->pool_alive) {
						if (Child *cand = this_sp->ready.pop()){
							cand->command(command,cand->name,arg...);
							//this_sp->register_pending(std::this_thread::get_id(),cand->name);
							//AtScopeEnd ase{[&](){this_sp->remove_pending(std::this_thread::get_id(),cand->name);}};
							return this_sp->waitOnChild(*cand);
						}
					} return nullptr;
				});
		}

		virtual ~ProcessPool_impl(){
			assert(this->pool_alive == false);
			typename decltype(children)::lock l{children.m};
			discard(l);
			for (auto &c : children.impl){
				if (c.name != 0)
					kill(c.name,SIGKILL);
			}
			std::cout << "process pool destroyed" << std::endl;
		}
	};

	template<typename Mem, typename Ret, typename... Arg>
	using ProcessPool = TaskPool<ProcessPool_impl<Mem, Ret,Arg...>,Mem, Ret,Arg...>;

}
