#pragma once
#include <memory>
#include <future>

//generic interface implemented by ProcessPool, ThreadPool, and NetworkedProcessPool.  

template<typename Ret, typename... Arg>
class TaskPool {

	//it is intended for the constructor to take an ordered list of tasks
public:	
	virtual std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg) = 0;

	virtual void print_pending() = 0;
	
	virtual ~TaskPool(){}

};
