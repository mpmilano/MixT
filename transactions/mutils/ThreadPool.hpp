#pragma once
#include "SafeSet.hpp"
#include "ctpl_stl.h"
#include "SerializationSupport.hpp"
#include <unistd.h>
#include <signal.h>
#include <exception>
#include "compile-time-tuple.hpp"
#include "TaskPool.hpp"
#include "utils.hpp"


//ThreadPool adheres to the TaskPool design restrictions, so that its interface matches ProcessPool.

namespace mutils{

	template<typename Ret, typename... Arg>
	class ThreadPool_impl : public TaskPool_impl<ThreadPool_impl<Ret,Arg...>,Ret,Arg...>{
		
		static void* hangOnToThis(void *v){
			static thread_local void* hang = v;
			if (v) hang = v;
			return hang;
		}
		
		typedef void* (*remember_t)(void*);
		const remember_t remember{&ThreadPool_impl::hangOnToThis};
	
	public:
	
		ThreadPool_impl (std::shared_ptr<ThreadPool_impl> &pp,
						  std::vector<std::function<Ret (std::function<void* (void*)>, int, Arg...)> > beh,
						  int limit,
						  std::function<Ret (std::exception_ptr)> onException
			):TaskPool_impl<ThreadPool_impl<Ret,Arg...>,Ret,Arg...>(pp,beh,limit,onException){}
		
		std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg){
			auto this_sp = this->this_sp;
			assert(this_sp.get() == this);
			return this->tp->push([this_sp,command,arg...](int id) -> std::unique_ptr<Ret>{
							try{
								return heap_copy(this_sp->behaviors.at(command)(this_sp->remember,id,arg...));
							}
							catch(...){
								return heap_copy(this_sp->onException(std::current_exception()));
							}
						});
		}

		virtual ~ThreadPool_impl(){
			assert(this->pool_alive == false);
			std::cout << "threadpool destroyed" << std::endl;
		}
	};
	
	template<typename Ret, typename... Arg>
	using ThreadPool = TaskPool<ThreadPool_impl<Ret,Arg...>,Ret,Arg...>;

}
