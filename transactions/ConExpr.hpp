#pragma once


#include "utils.hpp"
#include <string>
#include <memory>
#include <tuple>
#include "ConStatement.hpp"
#include "args-finder.hpp"

typedef Level Level;

template<typename T, Level l>
struct ConExpr : public ConStatement<l> {
	
	//typename std::conditional<l == Level::strong, T, void>::type
	//virtual strongCall(Store&, const Store&) const = 0;
	//virtual T causalCall(Store&, const Store&) const = 0;
};

template<Level l>
struct DummyConExpr : public ConExpr<void,l> {

	void strongCall(const Store&, const Store &) const {}

	void causalCall(const Store&, const Store &) const {}	

};

template<typename T, Level l>
constexpr bool is_ConExpr_f(const ConExpr<T,l>*){
	return true;
}

template<typename T>
constexpr bool is_ConExpr_f(const T*){
	return false;
}

template<typename Cls>
struct is_ConExpr : 
	std::integral_constant<bool, is_ConExpr_f(mke_p<Cls>()) || std::is_scalar<decay<Cls> >::value>::type {};

template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value)>
auto run_ast_strong(Store &c, const Store &s, const T& expr) {
	return expr.strongCall(c,s);
}

template<typename T>
typename std::enable_if<std::is_scalar<decay<T > >::value,T>::type
run_ast_strong(const Store &, const Store&, const T& e) {
	return e;
}

template<Level l, HandleAccess ha, typename T>
void markInTransaction(Store &s, const Handle<l,ha,T> &h){
	auto *ptr = dynamic_cast<DataStore<l>* >(&(h.remote_object().store()));
	assert(ptr);
	h.clone().t.markInTransaction(*ptr);
	s.insert(-1,&(h.clone().t));
}

template<HandleAccess ha, typename T>
void run_ast_strong(Store &c, const Store&, const Handle<Level::causal,ha,T>& h) {
	markInTransaction(c,h);
	//I think that this one, at least, is okay.
}
//*/
template<HandleAccess ha, typename T>
Handle<Level::strong,ha,T> run_ast_strong(Store &c, const Store&, const Handle<Level::strong,ha,T>& h) {
	markInTransaction(c,h);
	return h;
}


template<HandleAccess ha, typename T>
Handle<Level::strong,ha,T> run_ast_causal(Store &cache, const Store &s, const Handle<Level::strong,ha,T>& h) {
	markInTransaction(cache,h);
	struct LocalObject : public RemoteObject<T> {
		const T t;
		GDataStore &st;
		LocalObject(const T& t, GDataStore &st):t(t),st(st){}
		
		const T& get() const {return t;}
		void put(const T&) {
			assert(false && "error: modifying strong Handle in causal context! the type system is supposed to prevent this!");
		}
		bool isValid() const {
			//TODO: what if it's not valid? 
			return true;
		}
		const GDataStore& store() const {
			return st;
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
	return Handle<Level::strong,ha,T>{std::shared_ptr<LocalObject>{new LocalObject{cache.get<T>(h.uid),h.remote_object().store()}}};
	//TODO: need to cache this at the Transaction level!
	//TODO: need to ensure operations over strong handles
	//do not depend on causal data! (we probably already do this, but check!)

}


template<HandleAccess ha, typename T>
Handle<Level::causal,ha,T> run_ast_causal(Store &c, const Store &, const Handle<Level::causal,ha,T>& t) {
	markInTransaction(c,t);
	return t;
}
//*/

template<typename T, restrict(is_ConExpr<T>::value &&
							  !std::is_scalar<T>::value
							  && !is_handle<T>::value)>
auto run_ast_causal(Store &c, const Store &s, const T& expr) {
	return expr.causalCall(c,s);
}

template<typename T>
typename std::enable_if<std::is_scalar<decay<T > >::value,T>::type
run_ast_causal(const Store &, const Store&, const T& e) {
	return e;
}

template<typename T>
using run_result = decltype(run_ast_causal(std::declval<Store&>(),std::declval<Store&>(),std::declval<T&>()));

struct CacheLookupFailure {};

template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value)>
auto cached(const Store &cache, const T& ast){
	//TODO: make sure ast.id is always the gensym'd id.
	if (!cache.contains(ast.id)) throw CacheLookupFailure();
	using R = run_result<T>;
	return cache.get<R>(ast.id);
}

template<typename T>
type_check<std::is_scalar, T> cached(const Store &cache, const T& e){
	return e;
}


template<typename T, HandleAccess ha, Level l>
Handle<l,ha,T> cached(const Store &cache, const Handle<l,ha,T>& ast){
	return ast;
}

template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value)>
auto is_cached(const Store &cache, const T& ast){
	return cache.contains(ast.id);
}

template<typename T>
std::enable_if_t<std::is_scalar<T>::value, bool> is_cached(const Store &cache, const T& e){
	return true;
}


template<typename T, HandleAccess ha, Level l>
bool is_cached(const Store &cache, const Handle<l,ha,T>& ast){
	return true;
}


template<unsigned long long id, typename T>
std::enable_if_t<std::is_scalar<T>::value, std::nullptr_t> find_usage(const T&){
	return nullptr;
}

/*
template<unsigned long long id, typename T>
struct contains_temporary : std::false_type {
	static_assert(std::is_scalar<T>::value,"Error: you apparently didn't finish defining enough contains_temporaries");
};
*/

template<unsigned long long id>
struct contains_temporary<id,int> : std::false_type {};

template<unsigned long long id>
struct contains_temporary<id,bool> : std::false_type {};

