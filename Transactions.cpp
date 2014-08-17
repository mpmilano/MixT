#pragma once
#include "Client.hpp"
#include "extras"
#include "handle_utils"
#include "Transactions.hpp"

namespace backend {

	template<bool (*Lmatch) (Level), Level L, Client_Id cid, typename T>
	static constexpr 
	typename std::enable_if<Lmatch(L),bool>::type
	matches(DataStore::Handle<cid, L, HandleAccess::read, T> *){
			return true;
	}
	
	template<bool (*) (Level), typename C>
	static constexpr bool matches(C *){
		return false;
	}
	
	template<typename... Args>
	static constexpr Level handle_meet(){
		return exists(matches<is_causal> (make_nullptr<Args>())...) ? 
			Level::causal : Level::strong;
	}
		
	template<typename... Args>
	static constexpr Level handle_join(){
		return exists(matches<is_strong> (make_nullptr<Args>())...) ? 
			Level::strong : Level::causal;
	}

	template<Client_Id cid>
	Client<cid>::transaction_cls::transaction_cls(Client& c):c(c){
		c.sync_enabled = false;
	}

	template<Client_Id cid>
	template<Level l>
	void Client<cid>::transaction_cls::waitForSync(){
		c.sync_enabled = true;
		c.waitForSync<l>();
		c.sync_enabled = false;
	}	
	
	template<Client_Id cid>
	template < typename R, typename... Args>
	auto Client<cid>::transaction_cls::ro(R &f, Args... args) {
		static_assert(all_handles<Args...>::value, "Passed non-DataStore::Handles as arguments to function!");
		static_assert(!exists_write_handle<Args...>::value, "Passed write-enabled handles as argument to ro function!");
		static_assert(is_stateless<R, Client&, Args...>::value,
			      "You passed me a non-stateless function, or screwed up your arguments!");
		static_assert(is_stateless<R, Client&, Args...>::value,
			      "Expected: R f(DataStore&, DataStore::Handles....)");
		static_assert(all_handles_read<Args...>::value, "Error: passed non-readable handle into ro_transaction");
		waitForSync<handle_meet<Args...>()>();
		return f(c, args...);
	}

	template<Client_Id cid>
	template < typename R, typename... Args>
	void Client<cid>::transaction_cls::wo(R &f, Args... args) {
		static_assert(all_handles<Args...>::value, 
			      "Passed non-DataStore::Handles as arguments to function!");
		static_assert(!exists_read_handle<Args...>::value, 
			      "Passed read-enabled handles as argument to wo function!");
		static_assert(is_stateless<R, Client&, Args...>::value,
			      "You passed me a non-stateless function, or screwed up your arguments!");
		static_assert(is_stateless<R, Client&, Args...>::value,
			      "Expected: R f(DataStore&, DataStore::Handles....)");
		static_assert(all_handles_write<Args...>::value, "Error: passed non-writeable handle into wo_transaction");
		typename funcptr<R, Client&, Args...>::type f2 = f;
		static upfun f3 = [this,f2,args...](){
			f2(c,args...);
		};
		if (is_strong(handle_join<Args...>())) sync = true;
		c.pending_updates.run([&](typename pending::push_f &push){push(f3);});
		auto l = c.pending_updates.lock();
		f2(c, args...);
	}


	template<Client_Id cid>
	Client<cid>::transaction_cls::~transaction_cls(){
		c.sync_enabled = true;
		if (sync) c.waitForSync<Level::strong>();
		else c.waitForSync<Level::causal>();
		c.all_final = false;
	}


}
