
#pragma once
#include "Operation.hpp"
#include "Seq.hpp"


template<Level l, typename R>
struct Operate : ConStatement<l> {
	const int id;
	const std::function<R ()> f;
	const BitSet<HandleAbbrev> bs;
	const std::string name;
	Operate(const std::function<R ()>& f,
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
		s.get<std::function<R ()> >(id)();

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
struct PreOp {
	const T t;

	template<typename... Args>
	auto operator()(Args && ... args) const {
		//TODO: I'm sure there's some rationale behind
		//how exactly to measure this which is better.
		static constexpr Level l = min_level<Args...>::value;
		return Operate<l,decltype(t(args...))>
			([=]() mutable {return t(args...);},
			 BitSet<HandleAbbrev>::big_union(get_ReadSet(args)...),
			 type_name<typename T::F>()
				);
	}
};

//TODO - need to handle AST nodes in the argument list for this.

//if you really really want the error messages to be pretty, you
//can write a version of this that's overloaded up the wazoo on const
//vs non-const.
template<typename... Args>
auto _do_op(Operation<bool(*) (Args...)> (*fp) (Args...), Args... a){
	auto tmp = fp(a...);
	PreOp<decltype(tmp)> ret{tmp};
	return ret;
}

#define do_op2(Name, arg) _do_op(Name,extract_robj_p(arg))(arg)
#define do_op3(Name, arg1,arg2) _do_op(Name,extract_robj_p(arg1),extract_robj_p(arg2))(arg1,arg2)
#define do_op4(Name, arg1,arg2,arg3) _do_op(Name,extract_robj_p(arg1),extract_robj_p(arg2),extract_robj_p(arg3))(arg1,arg2,arg3)
#define do_op5(Name, arg1,arg2,arg3,arg4) _do_op(Name,extract_robj_p(arg1),extract_robj_p(arg2),extract_robj_p(arg3),extract_robj_p(arg4))(arg1,arg2,arg3,arg4)

#define do_op_IMPL2(count, ...) do_op ## count (__VA_ARGS__)
#define do_op_IMPL(count, ...) do_op_IMPL2(count, __VA_ARGS__)
#define do_op(...) do_op_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)
