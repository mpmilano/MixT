#pragma once
#include "Client.hpp"
#include "extras"
#include "handle_utils"
#include "Transactions.hpp"

namespace backend {
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
	template<Level Lmatch, typename T>
	constexpr auto Client<cid>::transaction_cls::matches(DataStore::Handle<cid, Lmatch, HandleAccess::read, T> *){
		return true;
	}

	template<Client_Id cid>
	template<Level, typename C>
	constexpr auto Client<cid>::transaction_cls::matches(C *){
		return false;
	}
	
	template<Client_Id cid>
	template<Level l, typename... Args>
	constexpr bool Client<cid>::transaction_cls::matches_any(){
		return exists(matches<l>(make_nullptr<Args>())...);
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
		waitForSync<matches_any<Level::causal, Args...>() ? Level::strong : Level::causal>();
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
		if (any_required_sync<Args...>::value) sync = true;
		c.pending_updates.run([&](typename pending::push_f &push){push(f3);});
		auto l = c.pending_updates.lock();
		f2(c, args...);
	}

	template<Client_Id cid>	
	template < typename R, typename... Args>
	auto Client<cid>::transaction_cls::rw(R &f, Args... args) {
		static_assert(all_handles<Args...>::value, "Passed non-DataStore::Handles as arguments to function!");
		static_assert(is_stateless<R, Client&, Args...>::value,
			      "You passed me a non-stateless function, or screwed up your arguments!");
		static_assert(is_stateless<R, Client&, Args...>::value,
			      "Expected: R f(DataStore&, DataStore::Handles....)");
		static_assert(!exists_rw_handle<Args...>::value, 
			      "ro and wo handles only please!");
		typename funcptr<R, Client&, Args...>::type f2 = f;
		static upfun f3 = [this,f2,args...](){
			f2(c,args...);
		};
		//read-sync at beginning if necessary (weakest)
		waitForSync<matches_any<Level::causal, Args...>() ? Level::strong : Level::causal>();
		
		//write-sync at the end if necessary (strongest)
		if (any_required_sync<Args...>::value) sync = true;
		
		c.pending_updates.run([&](typename pending::push_f &push){push(f3);});
		auto l = c.pending_updates.lock();
		return f2(c, args...);
	}

	template<Client_Id cid>
	Client<cid>::transaction_cls::~transaction_cls(){
		c.sync_enabled = true;
		if (sync) c.waitForSync<Level::strong>();
		else c.waitForSync<Level::causal>();
	}


}
