#pragma once
#include "Operation.hpp"
#include "Operate_macros.hpp"

template<Level l, typename R, typename Exprs>
struct Operate : ConStatement<l> {
	const int id;
	const std::function<R (Store &)> f;
	const BitSet<HandleAbbrev> bs;
	const std::string name;
	const Exprs exprs;
	Operate(const std::function<R (const Store&)>& f,
			const BitSet<HandleAbbrev> &bs,
			const std::string &name,
			const Exprs &exprs
		):
		id(gensym()),
		f(f),
		bs(bs),
		name(name),
		exprs(exprs){}

	auto getReadSet() const {
		return bs;
	}

	auto strongCall(Store &cache, const Store &s) const {
		std::integral_constant<bool,l==Level::strong>* choice = nullptr;
		return strongCall(cache,s,choice);
	}

	R strongCall(Store &cache, Store &, std::true_type*) const {
		//nothing causal, just do it all at once
		auto f2 = f; //need mutability, so can't be const copy.
		auto ret = f2(cache);
		cache.insert(id,ret);
		return ret;
	}

	void strongCall(Store &cache, Store &s, std::false_type*) const {
		//execute the strong expressions now. Remember they are supposed to be
		//self-caching
		fold(exprs,[&](const auto &e, bool){
				e.strongCall(cache,s);
				return false;},false);
	}


	auto causalCall(Store &cache, const Store &heap) const {
		std::integral_constant<bool,l==Level::causal>* choice = nullptr;
		return causalCall(cache,heap,choice);
	}	

	auto causalCall(Store &cache, const Store &heap, std::true_type*) const {
		//the function f assumes that absolutely everything will already be cached.
		//thus, we cannot call it until that's true.
		fold(exprs,[&](const auto &e, bool){
				e.causalCall(cache,heap);
				return false;},false);
		return f(cache);
	}

	R causalCall(Store &cache, const Store &, std::false_type*) const {
		//we were pure-strong, which means we're also already cached.
		assert(cache.contains(this->id));
		return cache.get<R>(this->id);
	}
};

template<unsigned long long ID, Level l, typename T, typename Vars>
auto find_usage(const Operate<l,T,Vars> &op){
	return fold(op.exprs,
				[](const auto &e, const auto &acc){
					if (!acc){
						return find_usage<ID>(e);
					}
					else return acc;
				}
				, nullptr);
}

template<unsigned long long ID, Level l, typename R, typename Exprs>
struct contains_temporary<ID, Operate<l,R,Exprs> > : contains_temp_fold<ID,Exprs > {};

template<Level l, typename i, typename E>
std::ostream & operator<<(std::ostream &os, const Operate<l,i,E>& op){
	return os << op.name;
}

template<typename T>
struct PreOp;

template<typename... J>
struct PreOp<std::tuple<J...> > {
	const std::tuple<J...> t;

	template<typename... Args>
	auto operator()(Args && ... args) const {
		//TODO: I'm sure there's some rationale behind
		//how exactly to measure this which is better.
		static constexpr Level l = min_level<Args...>::value;
		return Operate<l,decltype(std::get<0>(t)(run_ast(mke_store(),args)...)),decltype(std::make_tuple(args...)) >
			([=](const Store &c) mutable {
				std::pair<bool,bool> result =
					fold(t,[&](const auto &e, const std::pair<bool,bool> &acc){
							if (acc.first || !e.built_well) {
								return acc;
							}
							else {
								assert(e.built_well);
								return std::pair<bool,bool>(true,e(cached(c,args)...));
							}
						},std::pair<bool,bool>(false,false));
				assert(result.first && "Error: found no function to call");
				return result.second;
			},
			 BitSet<HandleAbbrev>::big_union(get_ReadSet(args)...),
				"This came from a tuple, so I don't know what to print",
				std::make_tuple(args...)
				);
	}
};

template<typename T>
auto make_PreOp(const T &t){
	PreOp<T> ret{t};
	return ret;
}

