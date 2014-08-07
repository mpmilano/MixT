#pragma once
#include "Client.hpp"
namespace backend{
		template<Client_Id cid>
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		Client<cid>::incr_op(DataStore::Handle<cid, L, HA, T> &h) 
			{
				upfun f = [h](){(*(h.hi().stored_obj))++;};
				std::function<void (typename pending::push_f&)> pf = 
					[&f](typename pending::push_f &push){push(f);};
				pending_updates.run(std::move(pf));
				f();
				waitForSync<L>();		
			}
		
		template<Client_Id cid>
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		Client<cid>::incr(DataStore::Handle<cid, L, HA, T> &h) {
			upfun f = [h](){ h.hi().stored_obj->incr(); };
			pending_updates.run([&f](typename pending::push_f &push){
					push(f);
				});
			f();
		}
		
		template<Client_Id cid>
		template<Level L, typename T, HandleAccess HA, typename F, typename... A>
		typename std::enable_if<canRead(HA), void>::type
		Client<cid>::add(DataStore::Handle<cid, L, HA, T> &h, F f2, A... args) {
			//todo - lifetime of args?
			upfun f = [h,f2,args...](){ f2(h.hi().stored_obj.get(),args...);};
			pending_updates.run([&f](typename pending::push_f &push){
					push(f);
				});
			f();
			waitForSync<L>();
		}

}