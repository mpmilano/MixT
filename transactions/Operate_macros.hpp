#pragma once

//TODO - why do I think this is ok?

#define trans_op_arg(x) constify([&](){Store c; Store s;				\
		auto f = x;														\
		typedef decltype(op_arg(run_ast_causal(c, s, f))) Res;			\
		Res r{};														\
		return r;}())

//TODO - need to handle AST nodes in the argument list for this.
#define do_op2(Name, arg) make_PreOp(Name(trans_op_arg(arg)))(arg)
#define do_op3(Name, arg1,arg2) make_PreOp(Name(trans_op_arg(arg1),trans_op_arg(arg2)))(arg1,arg2)

#define do_op_IMPL2(count, ...) do_op ## count (__VA_ARGS__)
#define do_op_IMPL(count, ...) do_op_IMPL2(count, __VA_ARGS__)
#define do_op(...) STANDARD_BEGIN(do_op_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__))
