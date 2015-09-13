#pragma once

#include "Operation.hpp"
#include "Operate_macros.hpp"
#include "ConExpr.hpp"
#include "Temporary.hpp"

template<Level l, typename R, typename Exprs>
struct Operate : ConStatement<l> {
	const int id;
	const std::function<R (const Cache &)> f;
	const Exprs exprs;
	Operate(const std::function<R (const Cache&)>& f,
			const Exprs &exprs,
			int id
		):
		id(id),
		f(f),
		exprs(exprs){}

	auto handles() const {
		return fold(exprs, [](const auto &e, const auto &acc){
				return std::tuple_cat(::handles(*e),acc);
			},std::tuple<>());
	}

	auto strongCall(StrongCache& cache, const StrongStore &s) const {
		choose_strong<l> choice{nullptr};
		return strongCall(cache,s,choice);
	}

	R strongCall(StrongCache& cache, const StrongStore &s, std::true_type*) const {
		//nothing causal, just do it all at once
		//need to cache things!
		strongCall(cache,s,(std::false_type*) nullptr);
		auto ret = f(cache);
		cache.insert(id,ret);
		return ret;
	}

	void strongCall(StrongCache& cache, const StrongStore &s, std::false_type*) const {
		//execute the strong expressions now. Remember they are supposed to be
		//self-caching
		fold(exprs,[&](const auto &e, bool){
				run_ast_strong(cache,s,*e);
				return false;},false);
	}


	auto causalCall(CausalCache& cache, const CausalStore &heap) const {
		choose_causal<l> choice{nullptr};
		return causalCall(cache,heap,choice);
	}	

	auto causalCall(CausalCache& cache, const CausalStore &heap, std::true_type*) const {
		//the function f assumes that absolutely everything will already be cached.
		//thus, we cannot call it until that's true.
		fold(exprs,[&](const auto &e, bool){
				run_ast_causal(cache,heap,*e);
				return false;},false);
		const Cache ro_cache{cache};
		return f(ro_cache);
	}

	R causalCall(CausalCache& cache, const CausalStore &, std::false_type*) const {
		//we were pure-strong, which means we're also already cached.
		assert(cache.contains(this->id));
		return cache.get<R>(this->id);
	}
};

template<unsigned long long ID, Level l, typename T, typename Vars>
auto find_usage(const Operate<l,T,Vars> &op){
	return fold(op.exprs,
				[](const auto &e, const auto &acc){
					return choose_non_np(acc,find_usage<ID>(*e));
				}
				, nullptr);
}


template<typename>
struct shared_deref_str;

template<typename T>
struct shared_deref_str<std::shared_ptr<T> > {
	using type = T;
};

template<typename T>
struct shared_deref_str<std::shared_ptr<T>&& > {
	using type = T;
};

template<typename T>
struct shared_deref_str<const std::shared_ptr<T>& > {
	using type = T;
};


template<typename T>
using shared_deref = typename shared_deref_str<T>::type;


template<unsigned long long ID, Level l, typename T, typename Temp>
auto cached_withfail(const Cache& c, const RefTemporary<ID,l,T,Temp> &rt){
	try {
		return cached(c,rt);
	}
	catch( const CacheLookupFailure&){
		std::cerr << "found a failure point (operate)!" << std::endl;
		std::cerr << "Type we failed on: RefTemporary<...>" << std::endl;		
		std::cout << "ID of temporary referenced: " << ID << std::endl;
		std::cout << "RefTemp ID referenced: " << rt.id << std::endl;
		std::cout << "RefTemp name referenced: " << rt.name << std::endl;
		std::cout << "address of cache: " << &c << std::endl;
		return cached(c,rt);
	}
}


template<typename T>
auto cached_withfail(const Cache& cache, const T &t){
	try {
		return cached(cache,t);
	}
	catch( const CacheLookupFailure&){
		std::cerr << "found a failure point (operate)!" << std::endl;
		std::cerr << "Type we failed on: " << type_name<T>() << std::endl;
		return cached(cache,t);
	}
}

template<typename T>
struct PreOp;

template<typename... J>
struct PreOp<std::tuple<J...> > {
	const int id;
	const std::tuple<J...> t;

	PreOp(int id, const std::tuple<J...> &t):id(id),t(t){
		assert(fold(t,[](const auto &e, bool acc){return e.built_well || acc;},false));
	}

	template<typename... Args>
	auto operator()(Args && ... args) const {
		//TODO: I'm sure there's some rationale behind
		//how exactly to measure this which is better.

		static constexpr Level l = min_level<shared_deref<Args>...>::value;
		assert(fold(t,[](const auto &e, bool acc){return e.built_well || acc;},false));
		auto t_ptr = shared_copy(t);
		assert(fold(*t_ptr,[](const auto &e, bool acc){return e.built_well || acc;},false));
		
		return Operate<l,decltype(std::get<0>(t)(
									  std::declval<run_result<shared_deref<decltype(args)> > >()...
									  )),decltype(std::make_tuple(args...)) >
			([=](const Cache& c) {
				assert(fold(*t_ptr,[](const auto &e, bool acc){return e.built_well || acc;},false));
				std::pair<bool,bool> result =
					fold(*t_ptr,[&](const auto &e, const std::pair<bool,bool> &acc){
							if (acc.first || !e.built_well) {
								return acc;
							}
							else {
								assert(e.built_well);
								return std::pair<bool,bool>(true,e(cached_withfail(c,*args)...));
							}
						},std::pair<bool,bool>(false,false));
				if (!result.first) throw NoOverloadFoundError{type_name<decltype(t)>()};
				return result.second;
			},
				std::make_tuple(args...),id
				);
	}
};

template<typename T>
auto make_PreOp(int id, const T &t){
	PreOp<T> ret{id,t};
	return ret;
}

template<typename T>
auto trans_op_arg(CausalCache& cs, const CausalStore& cc, const T& t){
	assert(false && "I think this is unsafe!");
	return constify(op_arg(run_ast_causal(cs,cc,op_arg(t))));
}

template<typename T>
auto trans_op_arg(StrongCache& , const StrongStore& , const T& ) ->
	decltype(constify(op_arg(std::declval<run_result<decltype(extract_robj_p(std::declval<T>()))> >()))) {
	assert(false && "I think this is unsafe!");
	
}
