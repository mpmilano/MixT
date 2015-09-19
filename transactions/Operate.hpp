#pragma once

#include "Operation.hpp"
#include "Operate_macros.hpp"
#include "ConExpr.hpp"
#include "Temporary.hpp"

/*
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
//*/

template<unsigned long long ID, Level l, typename T, typename Temp, StoreType st>
auto cached_withfail(const StoreMap<st>& c, const RefTemporary<ID,l,T,Temp> &rt){
	try {
		return cached(c,rt);
	}
	catch( const CacheLookupFailure&){
		std::cerr << "found a failure point (operate)!" << std::endl;
		std::cerr << "Type we failed on: RefTemporary<...>" << std::endl;		
		std::cerr << "ID of temporary referenced: " << ID << std::endl;
		std::cerr << "RefTemp ID referenced: " << rt.id << std::endl;
		std::cerr << "RefTemp name referenced: " << rt.name << std::endl;
		std::cerr << "address of cache: " << &c << std::endl;
		std::cerr << "Where it might be (strongCache): " << StrongCache::lost_and_found()[rt.id] << std::endl;
		std::cerr << "Where it might be (causalCache): " << CausalCache::lost_and_found()[rt.id] << std::endl;
		return cached(c,rt);
	}
}


template<typename T, StoreType st>
auto cached_withfail(const StoreMap<st>& cache, const T &t){
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
		static_assert(sizeof...(J) > 0, "Error: PreOp built with an empty operation!");
		assert(fold(t,[](const auto &e, bool acc){return e.built_well || acc;},false));
	}

	template<typename Cache, typename... Args>
	auto operator()(Cache &c, Args && ... args) const {
		assert(fold(t,[](const auto &e, bool acc){return e.built_well || acc;},false));
		auto t_ptr = shared_copy(t);
		assert(fold(*t_ptr,[](const auto &e, bool acc){return e.built_well || acc;},false));

		static_assert(is_Cache<std::decay_t<decltype(c)> >::value, "Error: This function can only take caches.");
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
	}
};

template<typename T>
auto make_PreOp(int id, const T &t){
	PreOp<T> ret{id,t};
	return ret;
}

template<typename T>
auto trans_op_arg(CausalCache& c, const CausalStore& s, const T& t) ->
	std::remove_reference_t<decltype(constify(op_arg(run_ast_causal(c,s,op_arg(t)))))>
{
	return constify(op_arg(run_ast_causal(c,s,t)));
}

template<typename T>
auto trans_op_arg(StrongCache& c, const StrongStore& s, const T& t) {
	run_ast_strong(c,s,t);
	return constify(extract_robj_p(cached(c,t)));
}
