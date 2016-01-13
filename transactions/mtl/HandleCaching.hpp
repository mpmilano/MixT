#pragma once
#include "Transaction.hpp"

namespace myria { namespace mtl {

		template<typename T>
		struct CachedObject : public RemoteObject<Level::strong,T> {
			std::shared_ptr<T> t;
			DataStore<Level::strong> &st;
			int nm;
			bool is_valid_only;
			CachedObject(decltype(t) t, DataStore<Level::strong> &st, Name name, bool is_valid_only)
				:t(std::move(t)),st(st),nm(name),is_valid_only(is_valid_only){}
	
			const T& get(TransactionContext *ctx) {
				assert(!is_valid_only);
				return *t;
			}
	
			void put(TransactionContext *ctx, const T&) {
				assert(false && "error: modifying strong Handle in causal context! the type system is supposed to prevent this!");
			}
	
			bool ro_isValid(TransactionContext *ctx) const {
				return t.get() || is_valid_only;
			}
	
			const DataStore<Level::strong>& store() const {
				return st;
			}

			Name name() const {
				return nm;
			}
	
			DataStore<Level::strong>& store() {
				return st;
			}
	
			int bytes_size() const {
				assert(false && "wait why are you ... stop!");
			}
	
			int to_bytes(char* v) const {
				assert(false && "wait why are you ... stop!");
			}
		
		};

		template<HandleAccess ha, typename T>
		Handle<Level::strong,ha,T> run_ast_strong(TransactionContext* tctx, const StrongCache& c, const StrongStore&, const Handle<Level::strong,ha,T>& _h) {

			auto ctx = context::current_context(c);
			auto h = _h.clone();

			assert(ctx != context::t::unknown);
			//TODO: this will do extra fetches.  Need to think about whether that's important. 

			if (ctx == context::t::read || ctx == context::t::validity){
				std::shared_ptr<T> ptr {nullptr};
				bool valid_only = false;
				if (ctx == context::t::read){
					ptr = mutils::heap_copy(h.get(tctx));
				}
				else if (ctx == context::t::validity){
					valid_only = h.isValid(tctx);
				}
				return 
					make_handle<Level::strong,ha,T,CachedObject<T> >
					(ptr,h.store(),h.name(),valid_only);
			}
			else return h;
		}

		template<typename T>
		struct LocalObject : public RemoteObject<Level::strong,T> {
			RemoteObject<Level::strong,T>& r;
			LocalObject(decltype(r) t)
				:r(t){}
			
			const T& get(TransactionContext *ctx) {
				//if the assert passes, then this was already fetched.
				//if the assert fails, then either we shouldn't have gone this deep
				//with causal calls,
				//or we have an info-flow violation,
				//or we can't infer the right level for operations (which would be weird).

				if(auto *co = dynamic_cast<CachedObject<T>* >(&r)){
					return co->get(ctx);
				}
				else assert(false && "Error: attempt get on non CachedObject. This should have been cached earlier");
			}
		
			void put(TransactionContext *ctx, const T&) {
				assert(false && "error: modifying strong Handle in causal context! the type system is supposed to prevent this!");
			}
		
			bool ro_isValid(TransactionContext *ctx) const {
				if(auto *co = dynamic_cast<CachedObject<T>* >(&r)){
					return co->ro_isValid(ctx);
				}
				else assert(false && "Error: attempt isValid on non CachedObject. This should have been cached earlier");
			}
		
			const DataStore<Level::strong>& store() const {
				return r.store();
			}
		
			Name name() const {
				return r.name();
			}
		
			DataStore<Level::strong>& store() {
				return r.store();
			}
		
			int bytes_size() const {
				assert(false && "wait why are you ... stop!");
			}
		
			int to_bytes(char* v) const {
				assert(false && "wait why are you ... stop!");
			}
		
		};	

		template<HandleAccess ha, typename T>
		Handle<Level::strong,ha,T> run_ast_causal(mtl::CausalCache& cache, const mtl::CausalStore &s, const Handle<Level::strong,ha,T>& h) {
			return 
				make_handle<Level::strong,ha,T,LocalObject<T> >
				(h.remote_object());
		}

	} }
