#pragma once

#define trans_op_arg(c,s,x) constify(op_arg(run_ast_causal(c, s, op_arg(x))))

#define do_op2(Name, _arg)												\
	[&](){																\
		auto a = _arg;													\
		using Arg = decltype(a);										\
		struct OperateImpl : public FindUsages<Arg>, public ConStatement<get_level<decltype(a)>::value >{ \
			const decltype(a) arg;										\
			const std::string name = #Name;								\
			OperateImpl(const decltype(arg) &a):FindUsages<Arg>(a),		\
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

#define do_op3(Name, _arg1,_arg2)										\
	[&](){																\
		auto a = _arg1;													\
		auto b = _arg2;													\
		using Arg1 = decltype(a);										\
		using Arg2 = decltype(b);										\
		struct OperateImpl :											\
		public FindUsages<Arg1,Arg2>,									\
		public ConStatement<min_level<decltype(a),decltype(b)>::value >	\
		{																\
			const decltype(a) arg1;										\
			const decltype(b) arg2;										\
			const std::string name = #Name;								\
			OperateImpl(const Arg1 &a, const Arg2 &b):					\
				FindUsages<Arg1,Arg2>(a,b),								\
				arg1(a),arg2(b){}										\
																		\
			BitSet<HandleAbbrev> getReadSet() const {					\
				assert(false && "what purpose does this serve?");		\
				return 0;												\
			}															\
																		\
			auto strongCall(Store &c CMA  const Store &s) const {		\
				return make_PreOp(Name(trans_op_arg(c, s, arg1),		\
									   trans_op_arg(c, s, arg2)))		\
					(arg1,arg2).strongCall(c CMA s);					\
			}															\
																		\
			auto causalCall(Store &c CMA  const Store &s) const {		\
				return make_PreOp(Name(trans_op_arg(c, s, arg1),		\
									   trans_op_arg(c, s, arg2)))		\
					(arg1,arg2).causalCall(c CMA s);					\
			}															\
																		\
		};																\
		OperateImpl r{a,b}; return r;									\
	} ()

#define do_op_IMPL2(count, ...) do_op ## count (__VA_ARGS__)
#define do_op_IMPL(count, ...) do_op_IMPL2(count, __VA_ARGS__)
#define do_op(...) STANDARD_BEGIN(do_op_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__))
