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

	template<typename, typename...>
	class ThreadPool;

	template<typename Ret, typename... Arg>
	class ThreadPool_impl{
		
		static void* hangOnToThis(void *v){
			static thread_local void* hang = v;
			if (v) hang = v;
			return hang;
		}
		
		typedef void* (*remember_t)(void*);
		const remember_t remember{&ThreadPool_impl::hangOnToThis};
		
		const int limit;
		std::unique_ptr<ctpl::thread_pool> tp;
		std::vector<std::function<Ret (std::function<void* (void*)>, int, Arg...)> > behaviors;
		std::function<Ret (std::exception_ptr)> onException;

		bool pool_alive;
		std::shared_ptr<ThreadPool_impl> &this_sp;
	
	public:

		friend class ThreadPool<Ret,Arg...>;
	
		ThreadPool_impl (std::shared_ptr<ThreadPool_impl> &pp,
						  std::vector<std::function<Ret (std::function<void* (void*)>, int, Arg...)> > beh,
						  int limit,
						  std::function<Ret (std::exception_ptr)> onException
			):limit(limit),tp(new ctpl::thread_pool{limit}),behaviors(beh),onException(onException),pool_alive(true),this_sp(pp){}

	private:
		SafeSet<std::pair<std::thread::id, int> > pending_set;
		void register_pending(std::thread::id id, int name){
			pending_set.add(std::make_pair(id,name));
		}
		void remove_pending(std::thread::id id, int name){
			pending_set.remove(std::make_pair(id,name));
		}
		
	public:

		void print_pending(){
			for (auto p : pending_set.iterable_copy()){
				std::cout << "thread id: " << p.first << " pid: " << p.second << std::endl;
			}
		}
		
		std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg){
			assert(this_sp.get() == this);
			auto this_sp = this->this_sp;
			return tp->push([this_sp,command,arg...](int id) -> std::unique_ptr<Ret>{
							try{
								return heap_copy(this_sp->behaviors.at(command)(this_sp->remember,id,arg...));
							}
							catch(...){
								return heap_copy(this_sp->onException(std::current_exception()));
							}
						});
		}

		virtual ~ThreadPool_impl(){
			assert(pool_alive == false);
			std::cout << "threadpool destroyed" << std::endl;
		}
	};
	
	template<typename Ret, typename... Arg>
	class ThreadPool : public TaskPool<Ret,Arg...> {
		std::shared_ptr<ThreadPool_impl<Ret,Arg...> > inst;
	public:
		ThreadPool (std::vector<std::function<Ret (std::function<void* (void*)>, int, Arg...)> > beh,
					 int limit = 200,
					 std::function<Ret (std::exception_ptr)> onExn = [](std::exception_ptr exn){
						 try {
							 assert(exn);
							 std::rethrow_exception(exn);
						 }
						 catch (...){
							 return default_on_exn<Ret>::value;
						 }
						 assert(false && "exn handler called with no currrent exception?");
					 }
			)
			:inst(new ThreadPool_impl<Ret,Arg...>(inst,beh,limit,onExn)){}

		std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg){
			return inst->launch(command,arg...);
		}

		void print_pending(){
			return inst->print_pending();
		}
	
		virtual ~ThreadPool(){
			inst->pool_alive = false;
			std::cout << "sent thread pool destroy" << std::endl;
		}
	};

}
