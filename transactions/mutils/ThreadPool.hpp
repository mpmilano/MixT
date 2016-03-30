#pragma once
#include "SafeSet.hpp"
#include "ctpl_stl.h"
#include "SerializationSupport.hpp"
#include <unistd.h>
#include <signal.h>
#include <exception>
#include <vector>
#include "compile-time-tuple.hpp"
#include "TaskPool.hpp"
#include "utils.hpp"


//ThreadPool adheres to the TaskPool design restrictions, so that its interface matches ProcessPool.

namespace mutils{

	template<typename Mem, typename Ret, typename... Arg>
	class ThreadPool_impl : public TaskPool_impl<ThreadPool_impl<Mem,Ret,Arg...>,Mem,Ret,Arg...>{

		std::vector<std::unique_ptr<Mem> > remember_these;
		std::unique_ptr<Mem> remember_this_single;
		
		SafeSet<int> indices;
	
	public:
	
		ThreadPool_impl (std::shared_ptr<ThreadPool_impl> &pp,
						 std::function<void (std::unique_ptr<Mem>&, int)> &init,
						  std::vector<std::function<Ret (std::unique_ptr<Mem>&, int, Arg...)> > beh,
						  int limit,
						  std::function<Ret (std::exception_ptr)> onException
			):TaskPool_impl<ThreadPool_impl<Mem,Ret,Arg...>,Mem,Ret,Arg...>(pp,init,beh,limit,onException),remember_these(this->tp ? limit : 1){
			for (int i = 0; i < (this->tp ? limit : 1); ++i)
				indices.add(i);

			for (auto i : indices.iterable_copy()){
				init(remember_these.at(i),i);
			}
		}
		
		std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg){
			auto this_sp = this->this_sp;
			assert(this_sp.get() == this);
			auto fun =
				[this_sp,command,arg...](int) -> std::unique_ptr<Ret>{
				auto mem_indx = this_sp->indices.pop();
				AtScopeEnd ase{[&](){this_sp->indices.add(mem_indx);}};
				try{
					return heap_copy(this_sp->behaviors.at(command)(this_sp->remember_these.at(mem_indx),mem_indx,arg...));
				}
				catch(...){
					return heap_copy(this_sp->onException(std::current_exception()));
				}
			};
			if (this->tp) return this->tp->push(fun);
			else {
				int id = std::hash<std::thread::id>{}(std::this_thread::get_id());
				return std::async(std::launch::deferred, std::move(fun),id);
			}
		}

		virtual ~ThreadPool_impl(){
			assert(this->pool_alive == false);
			std::cout << "threadpool destroyed" << std::endl;
		}
	};
	
	template<typename Mem, typename Ret, typename... Arg>
	using ThreadPool = TaskPool<ThreadPool_impl<Mem,Ret,Arg...>,Mem,Ret,Arg...>;

}
