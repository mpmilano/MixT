#pragma once
#include <memory>
#include <future>

//generic interface implemented by ProcessPool, ThreadPool, and NetworkedProcessPool.  

namespace mutils{

	template<typename>
	struct default_on_exn;
	
	template<>
	struct default_on_exn<std::string> {
		static constexpr auto value = "Exception Occurred!";
	};
	
	template<typename Impl, typename Mem, typename Ret, typename... Arg>
	class TaskPool;
	
	template<typename Impl, typename Mem, typename Ret, typename... Arg>
	class TaskPool_impl {

	protected:
		const int limit;
		std::unique_ptr<ctpl::thread_pool> tp;
		std::function<void (std::unique_ptr<Mem>&, int)> init;
		std::vector<std::function<Ret (std::unique_ptr<Mem>&, int, Arg...)> > behaviors;
		std::function<Ret (std::exception_ptr)> onException;
		bool pool_alive;
		std::shared_ptr<Impl> &this_sp;

		TaskPool_impl (std::shared_ptr<Impl> &pp,
					   std::function<void (std::unique_ptr<Mem>&, int)> &init,
					   std::vector<std::function<Ret (std::unique_ptr<Mem>&, int, Arg...)> > beh,
					   int limit,
					   std::function<Ret (std::exception_ptr)> onException
			):limit(limit),tp(limit > 0 ? new ctpl::thread_pool{limit} : nullptr),
			  init(init),behaviors(beh),onException(onException),pool_alive(true),this_sp(pp){}
		
		//it is intended for the constructor to take the same types as TaskPool
	public:
		virtual std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg) = 0;

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
		
		virtual ~TaskPool_impl(){}
		template<typename Mem2, typename Impl2, typename Ret2, typename... Arg2>
		friend class TaskPool;
	};
	
	template<class Impl, typename Mem, typename Ret, typename... Arg>
	class TaskPool {
		std::shared_ptr<Impl > inst;
	public:


		//The "memory" cell is guaranteed to be passed in queue order; we make the *longest*
		//possible duration elapse 

		TaskPool (
			std::function<void (std::unique_ptr<Mem>&, int)> init_mem,
			std::vector<std::function<Ret (std::unique_ptr<Mem>&, int, Arg...)> > beh,
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
			  })
			:inst(new Impl(inst,init_mem,beh,limit,onExn)){}
		
		std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg){
			return inst->launch(command,arg...);
		}
		
		void print_pending(){
			return inst->print_pending();
		}
		
		virtual ~TaskPool(){
			inst->pool_alive = false;
			std::cout << "sent task pool destroy" << std::endl;
		}
	};
	
}
