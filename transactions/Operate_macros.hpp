#pragma once

#define trans_op_arg(c,s,x) constify(op_arg(run_ast_causal(c, s, op_arg(x))))

#define do_op2(Name, _arg)												\
	[&](){																\
		auto a = _arg;													\
		using Arg = decltype(a);										\
		struct OperateImpl : public FindUsages<Arg>, public ConStatement<get_level<decltype(a)>::value >{ \
			const decltype(a) arg;										\
			OperateImpl(const decltype(arg) &a):FindUsages<Arg>(a), \
				arg(a){}												\
																		\
			BitSet<HandleAbbrev> getReadSet() const {					\
				assert(false && "what purpose does this serve?");		\
				return 0;												\
			}															\
																		\
			auto strongCall(Store &c CMA  const Store &s) const {		\
				return make_PreOp(Name(trans_op_arg(c, s, arg)))		\
					(arg).strongCall(c CMA s);							\
			}															\
																		\
			auto causalCall(Store &c CMA  const Store &s) const {		\
				return make_PreOp(Name(trans_op_arg(c, s, arg)))		\
					(arg).causalCall(c CMA s);							\
			}															\
		};																\
		OperateImpl r{a}; return r;										\
	} ()


#define do_op_IMPL2(count, ...) do_op ## count (__VA_ARGS__)
#define do_op_IMPL(count, ...) do_op_IMPL2(count, __VA_ARGS__)
#define do_op(...) STANDARD_BEGIN(do_op_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__))
