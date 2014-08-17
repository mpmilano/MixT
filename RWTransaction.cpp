#pragma once

namespace backend{

	template<Client_Id cid>
	class Client<cid>::transaction_cls::rw_cls {
		typedef int Tid;
		template<Level L, typename T, Tid... depends>
		class ReadRes{
		private:
			std::shared_ptr<std::function<T ()> > thunk;
			ReadRes(std::function<T ()> thunk):thunk(thunk){}
		public:
			template<Level L_, typename F, Tid... deps2>
			//we can also change the level here.
			//right now we're just deflating it when needed.
			ReadRes<meet(L,L_),T,depends..., deps2...>
			a(F f, ReadRes<L_,T,deps2...> gnu){
				auto thunk = this->thunk;
				return ReadRes<meet(L,L_),T,depends..., deps2...> (
					[f,thunk,gnu](){
						return f((*thunk)(), gnu.thunk()); 
					});
			}
			
			template<Level L_, typename T_, Tid... others>
			friend class ReadRes;
		};
		
		template<Level L, Tid... allowed>
		class TransWrite{
		private:
			transaction_cls t;
			TransWrite(transaction_cls t):t(t){}
		public:
			template<Level L_, typename F, typename T, Tid... depends>
			typename std::enable_if<subset<Tid>::f(subset<Tid>::pack<depends...>(), subset<Tid>::pack<allowed...>()) >::type
			//note - we can enforce a predicate on level here!
			//We're not going to though.
			write(F f, ReadRes<L_,T,depends...> rr) {
				t.waitForSync<L_>(); //<-- what's the correctness condition on when this happens?
				//wait for READ sync.
				//acquire warranty on read value.
				
				//if (L == Level::strong) t.sync = true; <-- you don't need to do this here if the rw transaction itself is doing it.
				f((*rr.thunk)());
			}
			//it doesn't make a lot of sense to talk about dividing a transaction up the way andrew was talking about.
			//we've got a core problem - the way the consistency model is phrased, for two reads to be independent of each-other
			// (as in they affect different writes) and for that to therefore allow us to read at a different consistency level
			// means bad things.  Either the reads and writes are related, in which case we've just violated isolation (you have 
			// new data on the one hand and old data on the other) or your reads and writes really aren't related, in which case 
			// why are they in the same transaction?
			
			//moreover, the current model doesn't support this mode of thinking at all.  Sync() is a single operation - you bring
			// your _entire_ object state up to date in one fell swoop, and you additionally push your _own_ state back to the server.
			// even if they were separated, then a single high-level read would have the effect of making all subsequent reads de-facto
			// high-level.  This isn't actually going to result in the "false strong" problem - the subsequent (weaker) reads will stil
			// be _reported_ as lower-consistency, so the fact that there may be pending outstanding operations that the cache-sync didn't
			// catch is allowable under the model.
			
			// So the real problem here is that under the current model, the order in which you happen to read will potentially affect your 
			// correctness and performance (ie release partial transaction to master).
			
			// we can fix the correctness issue by de-coupling local fast-forward from master-update (not obvious how to do this, but it can be
			// done), but the performance issue still exists.  Is this really a problem? It won't affect the overall performance of a transaction,
			// after all.  
			
			// thing to look up - what's supposed to happen if you have a strong read, followed by someone else updating the value, followed by a strong
			// write?  Should the transaction abort and now be re-tried, or should it just go ahead anyway?
			
			//so summary - you can talk about propagating taint through a transaction to figure out which reads flow into which writes (and more importantly 
			//which reads flow into each other), and only calculate the join (meet? the makes-it-lower one) of the reads that interact.  You can trigger a 
			//sync operation only on the strong reads (there should be few of these), and while that does mean that you are partially-syncing the weak reads
			//that follow, that doesn't violate anyone's correctness or performance.  You _do_ need to de-couple client-pulls from server-pushs, so that you
			//only do the latter at the _end_ of transactions.
				
		};
			
		template<Level L, typename T, Tid from>
		class TransRead{
		public:
			operator ReadRes<L,T,from> ();
		};

	};

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
		waitForSync<handle_meet<Args...>()>();
		
		//write-sync at the end if necessary (strongest)
		if (is_strong(handle_join<Args...>())) sync = true;
		
		c.pending_updates.run([&](typename pending::push_f &push){push(f3);});
		auto l = c.pending_updates.lock();
		return f2(c, args...);
	}
}

