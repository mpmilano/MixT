#pragma once

#define trans_op_arg(c,s,x) constify(op_arg(run_ast_causal(c, s, op_arg(x))))

#define do_op2(Name, _arg)												\
	[&](){																\
		auto a = _arg;													\
		using Arg = decltype(a);										\
		struct OperateImpl : public FindUsages<Arg>, public ConStatement<get_level<decltype(a)>::value >{ \
			const int id = gensym();									\
			const std::shared_ptr<decltype(a)> arg;						\
			const std::string name = #Name;								\
			OperateImpl(const decltype(a) &a):FindUsages<Arg>(a),		\
				arg(shared_copy(a)){}									\
																		\
			auto handles() const {										\
				auto ret = ::handles(*arg);								\
				assert(std::tuple_size<decltype(ret)>::value > 0);		\
				return ret;												\
																		\
			}															\
																		\
			auto strongCall(Store &c CMA  const Store &s) const {		\
				return make_PreOp(id,Name(trans_op_arg(c, s, *arg)))	\
					(arg).strongCall(c CMA s);							\
			}															\
																		\
			auto causalCall(Store &c CMA  const Store &s) const {		\
				return make_PreOp(id,Name(trans_op_arg(c, s, *arg)))	\
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
			const int id = gensym();									\
			const std::shared_ptr<decltype(a)> arg1;					\
			const std::shared_ptr<decltype(b)> arg2;					\
			const std::string name = #Name;								\
			OperateImpl(const Arg1 &a, const Arg2 &b):					\
				FindUsages<Arg1,Arg2>(a,b),								\
				arg1(shared_copy(a)),arg2(shared_copy(b)){}				\
																		\
			auto handles() const {										\
				auto ret =  std::tuple_cat								\
					(::handles(*arg1),::handles(*arg2));				\
				assert(std::tuple_size<decltype(ret)>::value > 0);		\
				std::cout << "COLLECTED THESE HANDLES: " << ret << std::endl; \
				return ret;												\
			}															\
																		\
			auto strongCall(Store &c CMA  const Store &s) const {		\
				return make_PreOp(id,Name(trans_op_arg(c, s, *arg1),	\
									   trans_op_arg(c, s, *arg2)))		\
					(arg1,arg2).strongCall(c CMA s);					\
			}															\
																		\
			auto causalCall(Store &c CMA  const Store &s) const {		\
				return make_PreOp(id,Name(trans_op_arg(c, s, *arg1),	\
									   trans_op_arg(c, s, *arg2)))		\
					(arg1,arg2).causalCall(c CMA s);					\
			}															\
																		\
		};																\
		OperateImpl r{a,b}; return r;									\
	} ()

#define do_op_IMPL2(count, ...) do_op ## count (__VA_ARGS__)
#define do_op_IMPL(count, ...) do_op_IMPL2(count, __VA_ARGS__)
#define do_op(...) STANDARD_BEGIN(do_op_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__))
