
#pragma once
#include "Operation.hpp"
#include "Operate_macros.hpp"

template<Level l, typename R>
struct Operate : ConStatement<l> {
	const int id;
	const std::function<R (Store &)> f;
	const BitSet<HandleAbbrev> bs;
	const std::string name;
	Operate(const std::function<R (Store &)>& f,
			const BitSet<HandleAbbrev> &bs,
			const std::string name):
		id(gensym()),
		f(f),
		bs(bs),
		name(name){}

	auto getReadSet() const {
		return bs;
	}

	auto operator()(Store &s) const {
		s.insert(id,f);
		s.get<std::function<R (Store &)> >(id)(s);

		//TODO: expose failure chances? 
		return true;
	}
};

template<Level l, typename T>
auto find_usage(const Operate<l,T> &){
	assert(false && "this needs to be available in operate? geez.");
	return nullptr;
}

template<Level l, typename i>
std::ostream & operator<<(std::ostream &os, const Operate<l,i>& op){
	return os << op.name;
}

template<typename T>
struct PreOp;


template<typename T, restrict(is_handle<decay<T> >::value)>
auto run_ast(const Store &, T && t) {
	return std::forward<T>(t);
}


template<typename... J>
struct PreOp<std::tuple<J...> > {
	const std::tuple<J...> t;

	template<typename... Args>
	auto operator()(Args && ... args) const {
		//TODO: I'm sure there's some rationale behind
		//how exactly to measure this which is better.
		static constexpr Level l = min_level<Args...>::value;
		return Operate<l,decltype(std::get<0>(t)(run_ast(mke_store(),args)...))>
			([=](Store &s) mutable {
				std::pair<bool,bool> result =
					fold(t,[&](const auto &e, const std::pair<bool,bool> &acc){
							if (acc.first || !e.built_well) {
								return acc;
							}
							else {
								assert(e.built_well);
								//static auto _run_ast = [&](auto && e){return run_ast(s,e);};
								return std::pair<bool,bool>(true,e(run_ast(s,args)...));
							}
						},std::pair<bool,bool>(false,false));
				assert(result.first && "Error: found no function to call");
				return result.second;
			},
			 BitSet<HandleAbbrev>::big_union(get_ReadSet(args)...),
			 "This came from a tuple, so I don't know what to print"
				);
	}
};

template<typename T>
auto make_PreOp(const T &t){
	PreOp<T> ret{t};
	return ret;
}

