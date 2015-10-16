#pragma once
#include "Transaction.hpp"

template<typename T>
struct CachedObject : public RemoteObject<T> {
	std::shared_ptr<T> t;
	GDataStore &st;
	std::string nm;
	bool is_valid_only;
	CachedObject(decltype(t) t, GDataStore &st, const std::string &name, bool is_valid_only)
		:t(std::move(t)),st(st),nm(name),is_valid_only(is_valid_only){}
	
	TransactionContext* currentTransactionContext(){
		assert(false && "you probably didn't mean to call this");
	}
	
	void setTransactionContext(TransactionContext* tc){
		assert(false && "you probably didn't mean to call this");
	}	
	
	const T& get() {
		assert(!is_valid_only);
		return *t;
	}
	
	void put(const T&) {
			assert(false && "error: modifying strong Handle in causal context! the type system is supposed to prevent this!");
	}
	
	bool ro_isValid() const {
		return t.get() || is_valid_only;
	}
	
	const GDataStore& store() const {
		return st;
	}

	const std::string& name() const {
		return nm;
	}
	
	GDataStore& store() {
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
Handle<Level::strong,ha,T> run_ast_strong(const StrongCache& c, const StrongStore&, const Handle<Level::strong,ha,T>& _h) {

	auto ctx = context::current_context(c);
	auto h = _h.clone();

	assert(ctx != context::t::unknown);
	//TODO: this will do extra fetches.  Need to think about whether that's important. 

	if (ctx == context::t::read || ctx == context::t::validity){
		std::shared_ptr<T> ptr {nullptr};
		bool valid_only = false;
		if (ctx == context::t::read){
			ptr = heap_copy(h.get());
		}
		else if (ctx == context::t::validity){
			valid_only = h.isValid();
		}
		return 
			make_handle<Level::strong,ha,T,CachedObject<T> >
			(ptr,h.store(),h.name(),valid_only);
	}
	else return h;
}

template<typename T>
	struct LocalObject : public RemoteObject<T> {
		RemoteObject<T>& r;
		LocalObject(decltype(r) t)
			:r(t){}
		
		TransactionContext* currentTransactionContext(){
			assert(false && "you probably didn't mean to call this");
		}
		
		void setTransactionContext(TransactionContext* tc){
			assert(false && "you probably didn't mean to call this");
		}	
		
		const T& get() {
			//if the assert passes, then this was already fetched.
			//if the assert fails, then either we shouldn't have gone this deep
			//with causal calls,
			//or we have an info-flow violation,
			//or we can't infer the right level for operations (which would be weird).

			if(auto *co = dynamic_cast<CachedObject<T>* >(&r)){
				return co->get();
			}
			else assert(false && "Error: attempt get on non CachedObject. This should have been cached earlier");
		}
		
		void put(const T&) {
			assert(false && "error: modifying strong Handle in causal context! the type system is supposed to prevent this!");
		}
		
		bool ro_isValid() const {
			if(auto *co = dynamic_cast<CachedObject<T>* >(&r)){
				return co->ro_isValid();
			}
			else assert(false && "Error: attempt isValid on non CachedObject. This should have been cached earlier");
		}
		
		const GDataStore& store() const {
			return r.store();
		}
		
		const std::string& name() const {
			return r.name();
		}
		
		GDataStore& store() {
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
Handle<Level::strong,ha,T> run_ast_causal(CausalCache& cache, const CausalStore &s, const Handle<Level::strong,ha,T>& h) {
	return 
		make_handle<Level::strong,ha,T,LocalObject<T> >
		(h.remote_object());
}
