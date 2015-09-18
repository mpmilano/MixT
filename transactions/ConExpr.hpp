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
	//virtual strongCall(Cache&, const Store&) const = 0;
	//virtual T causalCall(Cache&, const Store&) const = 0;
};


template<typename T, restrict(! (is_ConStatement<T>::value || is_handle<T>::value || is_tuple<T>::value || (is_ConExpr<T>::value && !std::is_scalar<T>::value)))>
constexpr Level get_level_f(const T*){
	return Level::undef;
}

template<typename T, Level l>
constexpr Level get_level_f(const ConExpr<T,l>*){
	return l;
}

//imported from ConStatement. Probably should get its own file or something.
template<typename T>
struct get_level : std::integral_constant<Level, get_level_f(mke_p<T>())>::type {};

template<typename T, Level l>
constexpr Level chld_min_level_f(ConExpr<T,l> const * const){
	return l;
}

template<typename T, restrict(std::is_scalar<T>::value)>
constexpr Level chld_min_level_f(T const * const t){
	return get_level_f(t);
}

template<Level l>
constexpr Level chld_min_level_f(level_constant<l> const * const ){
	return l;
}

template<typename T>
struct chld_min_level : level_constant<chld_min_level_f(mke_p<T>())> {};

template<typename T>
struct chld_min_level<const T> : chld_min_level<T> {};

template<typename... T>
struct min_level : std::integral_constant<Level,
										  (exists((chld_min_level<T>::value == Level::causal)...) ?
										   Level::causal :
										   (exists((chld_min_level<T>::value == Level::strong)...) ? Level::strong : Level::undef )
										   )>::type {};

template<typename T, restrict(std::is_scalar<T>::value)>
constexpr Level get_level_dref(T*){
	return Level::undef;
}

template<HandleAccess ha, Level l, typename T>
constexpr Level get_level_dref(Handle<l,ha,T> const * const){
	return l;
}

template<typename T, Level l>
constexpr Level get_level_dref(ConExpr<T,l> *){
	return (l == Level::causal ? Level::causal : get_level_dref(mke_p<T>()));
}


template<typename... T>
struct min_level_dref : std::integral_constant<Level,
											   exists((get_level_dref(mke_p<T>()) == Level::causal)...) ? Level::causal :
											   (exists((get_level_dref(mke_p<T>()) == Level::strong)...) ? Level::strong :
												Level::undef)
											   > {
	static_assert(!exists((!is_ConExpr<T>::value)...),"Error: min_level_dref only ready for Exprs. Not sure why you need this for non-exprs...");
};

template<typename... T>
struct max_level_dref : std::integral_constant<Level,
											   exists((get_level_dref(mke_p<T>()) == Level::strong)...) ? Level::strong :
											   (exists((get_level_dref(mke_p<T>()) == Level::causal)...) ? Level::causal :
												Level::undef)
											   > {
	static_assert(!exists((!is_ConExpr<T>::value)...),"Error: max_level_dref only ready for Exprs. Not sure why you need this for non-exprs...");
};

template<typename... T>
struct min_level<std::tuple<T...> > : min_level<T...> {};

template<typename T>
struct chld_max_level : get_level<T>{};

template<typename T>
struct chld_max_level<const T> : chld_max_level<T> {};

template<typename... T>
struct max_level : std::integral_constant<Level,
										  (exists((chld_max_level<T>::value == Level::strong)...) ?
										   Level::strong :
										   (exists((chld_max_level<T>::value == Level::causal)...) ?
											Level::causal : Level::undef))>::type {};

template<typename... T>
struct max_level<std::tuple<T...> > : max_level<T...> {};


template<Level l>
struct DummyConExpr : public ConExpr<void,l> {

	std::tuple<> handles() const {
		static std::tuple<> ret;
		return ret;
	}
	
	void strongCall(const StrongCache&, const StrongStore &) const {}

	void causalCall(const CausalCache&, const CausalStore &) const {}	

};

template<typename T, Level l>
constexpr bool is_ConExpr_f(ConExpr<T,l> const * const){
	return true;
}

template<typename T>
constexpr bool is_ConExpr_f(const T*){
	return false;
}

template<typename Cls>
struct is_ConExpr : 
	std::integral_constant<bool, is_ConExpr_f(mke_p<Cls>()) || std::is_scalar<decay<Cls> >::value>::type {};

template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value && !is_handle<T>::value)>
auto run_ast_strong(StrongCache& c, const StrongStore &s, const T& expr) {
	return expr.strongCall(c,s);
}

template<typename T>
typename std::enable_if<std::is_scalar<decay<T > >::value,T>::type
run_ast_strong(const StrongCache &, const StrongStore&, const T& e) {
	return e;
}

template<HandleAccess ha, typename T>
void run_ast_strong(const StrongCache& c, const StrongStore&, const Handle<Level::causal,ha,T>& h) {
	//I think that this one, at least, is okay.
}
//*/
template<HandleAccess ha, typename T>
Handle<Level::strong,ha,T> run_ast_strong(const StrongCache& c, const StrongStore&, const Handle<Level::strong,ha,T>& h) {
	return h;
}


template<HandleAccess ha, typename T>
Handle<Level::strong,ha,T> run_ast_causal(CausalCache& cache, const CausalStore &s, const Handle<Level::strong,ha,T>& h) {
	struct LocalObject : public RemoteObject<T> {
		const T t;
		GDataStore &st;
		TransactionContext *tc;
		LocalObject(const T& t, GDataStore &st, TransactionContext* tc)
			:t(t),st(st),tc(tc){}

		TransactionContext* currentTransactionContext(){
			assert(false && "you probably didn't mean to call this");
			return tc;
		}

		void setTransactionContext(TransactionContext* tc){
			assert(false && "you probably didn't mean to call this");
			this->tc = tc;
		}
		
		const T& get() {return t;}
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
	return Handle<Level::strong,ha,T>{
		std::shared_ptr<LocalObject>{
			new LocalObject{cache.get<T>(h.uid),
					h.remote_object().store(),
					h.remote_object().currentTransactionContext()}}};
	//TODO: need to cache this at the Transaction level!
	//TODO: need to ensure operations over strong handles
	//do not depend on causal data! (we probably already do this, but check!)

}


template<HandleAccess ha, typename T>
Handle<Level::causal,ha,T> run_ast_causal(const CausalCache& c, const CausalStore &, const Handle<Level::causal,ha,T>& t) {
	return t;
}
//*/

template<typename T, restrict(is_ConExpr<T>::value &&
							  !std::is_scalar<T>::value
							  && !is_handle<T>::value)>
auto run_ast_causal(CausalCache& c, const CausalStore &s, const T& expr) {
	return expr.causalCall(c,s);
}

template<typename T>
typename std::enable_if<std::is_scalar<decay<T > >::value,T>::type
run_ast_causal(const CausalCache &, const CausalStore&, const T& e) {
	return e;
}

template<typename T>
using run_result = decltype(run_ast_causal(std::declval<CausalCache&>(),std::declval<CausalStore&>(),std::declval<T&>()));

struct CacheLookupFailure {};

template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value)>
auto cached(const StrongCache& cache, const T& ast){
	//TODO: make sure ast.id is always the gensym'd id.
	if (!cache.contains(ast.id)) throw CacheLookupFailure();
	using R = run_result<T>;
	return cache.get<R>(ast.id);
}

template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value)>
auto cached(const CausalCache& cache, const T& ast){
	//TODO: make sure ast.id is always the gensym'd id.
	if (!cache.contains(ast.id)) throw CacheLookupFailure();
	using R = run_result<T>;
	return cache.get<R>(ast.id);
}

template<typename T, StoreType st>
type_check<std::is_scalar, T> cached(const StoreMap<st>& cache, const T& e){
	return e;
}


template<typename T, HandleAccess ha, Level l, StoreType st>
Handle<l,ha,T> cached(const StoreMap<st>& cache, const Handle<l,ha,T>& ast){
	return ast;
}

template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value)>
auto is_cached(const CausalCache& cache, const T& ast){
	return cache.contains(ast.id);
}

template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value)>
auto is_cached(const StrongCache& cache, const T& ast){
	return cache.contains(ast.id);
}

template<typename T, StoreType st>
std::enable_if_t<std::is_scalar<T>::value, bool> is_cached(const StoreMap<st>& cache, const T& e){
	return true;
}


template<typename T, HandleAccess ha, Level l, StoreType st>
bool is_cached(const StoreMap<st>& cache, const Handle<l,ha,T>& ast){
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

template<Level l, HandleAccess ha, typename T>
auto handles(const Handle<l,ha,T>& h) {
	return std::make_tuple(h);
}

template<typename T>
std::enable_if_t<std::is_scalar<T>::value, std::tuple<> > handles(const T&){
	return std::tuple<>();
}


template<typename T, restrict(is_ConExpr<T>::value && !std::is_scalar<T>::value)>
auto handles(const T &e){
	return e.handles();
}

template<typename... T>
auto handles(const std::tuple<T...> &params){
	return fold(params,
				[](const auto &e, const auto &acc){
					return std::tuple_cat(::handles(e),acc);
				}
				,std::tuple<>());
}

template<typename T>
using is_AST_Expr = typename std::integral_constant<
	bool,
	is_ConExpr<T>::value &&
	!std::is_scalar<T>::value &&
	!is_handle<T>::value >::type;
