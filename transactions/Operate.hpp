
#pragma once
#include "Operation.hpp"
#include "Seq.hpp"


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

template<Level l, typename R>
constexpr bool verify_compilation_complete(const Operate<l,R>*){
	return true;
}

template<Level l, typename i>
std::ostream & operator<<(std::ostream &os, const Operate<l,i>& op){
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
		return Operate<l,decltype(std::get<0>(t)(args...))>
			([=](Store &) mutable {
				std::pair<bool,bool> result =
					fold(t,[&](const auto &e, const std::pair<bool,bool> &acc){
							if (acc.first || !e.built_well) {
								return acc;
							}
							else {
								assert(e.built_well);
								return std::pair<bool,bool>(true,e(args...));
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

//TODO - need to handle AST nodes in the argument list for this.
#define do_op2(Name, arg) make_PreOp(Name(op_arg(arg)))(arg)
#define do_op3(Name, arg1,arg2) make_PreOp(Name(op_arg(arg1),op_arg(arg2)))(arg1,arg2)

#define do_op_IMPL2(count, ...) do_op ## count (__VA_ARGS__)
#define do_op_IMPL(count, ...) do_op_IMPL2(count, __VA_ARGS__)
#define do_op(...) do_op_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)
