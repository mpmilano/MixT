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
	const int id = gensym();
	//typename std::conditional<l == Level::strong, T, void>::type
	//virtual strongCall(Store&, const Store&) const = 0;
	//virtual T causalCall(Store&, const Store&) const = 0;
};

template<Level l>
struct DummyConExpr : public ConExpr<void,l> {

	void strongCall(const Store&, const Store &) const {}

	void causalCall(const Store&, const Store &) const {}
	
	BitSet<HandleAbbrev> getReadSet() const {
		return BitSet<HandleAbbrev>();
	}
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

template<typename Expr, restrict(is_ConExpr<Expr>::value && std::is_scalar<Expr>::value)>
auto get_ReadSet(const Expr &){
	return BitSet<HandleAbbrev>();
}

template<typename T>
typename std::enable_if<is_ConStatement<T>::value && !std::is_scalar<T>::value,
						BitSet<HandleAbbrev> >::type get_ReadSet(const T &ce){
	assert(&ce != nullptr);
	return ce.getReadSet();
}

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
Handle<Level::strong,ha,T> run_ast_causal(const Store &cache, const Store &s, const Handle<Level::strong,ha,T>& h) {
	markInTransaction(cache,h);
	struct LocalObject : public RemoteObject<T> {
		const T t;
	};
	return Handle<Level::strong,ha,T>{std::shared_ptr<LocalObject>{new LocalObject{cache.get<T>(h.uid)}}};
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

template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value)>
auto run_ast_causal(Store &c, const Store &s, const T& expr) {
	return expr.causalCall(c,s);
}

template<typename T>
typename std::enable_if<std::is_scalar<decay<T > >::value,T>::type
run_ast_causal(const Store &, const Store&, const T& e) {
	return e;
}

template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value)>
auto cached(const Store &cache, const T& ast){
	using R = decltype(run_ast_causal(mke_store(),cache,ast));
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


template<unsigned long long id, typename T>
enable_if<std::is_scalar<T>::value, std::nullptr_t> find_usage(const T&){
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
