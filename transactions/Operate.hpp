
#pragma once
#include "Operation.hpp"
#include "Seq.hpp"

//TODO - need to handle AST nodes in the argument list for this.


template<typename... Args>
auto _do_op(Operation<bool(*) (Args...)> (*fp) (Args...), Args... a){
	return fp(a...);
}

#define do_op2(Name, arg) _do_op(Name,extract_robj_p(arg))(arg)
#define do_op3(Name, arg1,arg2) _do_op(Name,extract_robj_p(arg1,arg2))(arg1,arg2)
#define do_op4(Name, arg1,arg2,arg3) _do_op(Name,extract_robj_p(arg1,arg2,arg3))(arg1,arg2,arg3)
#define do_op5(Name, arg1,arg2,arg3,arg4) _do_op(Name,extract_robj_p(arg1,arg2,arg3,arg4))(arg1,arg2,arg3,arg4)

#define do_op_IMPL2(count, ...) do_op ## count (__VA_ARGS__)
#define do_op_IMPL(count, ...) do_op_IMPL2(count, __VA_ARGS__)
#define do_op(...) do_op_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)
