#pragma once
#include "macro_utils.hpp"

//#define trans_op_arg(c,s,x) constify(op_arg(run_ast_causal(c, s, op_arg(x))))

#define alphabet1 a
#define alphabet2 a, b
#define alphabet3 a, b, c
#define alphabet(n) alphabet ## n

#define alphabet_assign1(x) auto a = x;
#define alphabet_assign2(x,y) auto a = x; auto b = y;
#define alphabet_assign3(x,y,z) auto a = x; auto b = y; auto c = z;

#define alphabet_assign_IMPL2(count, ...) alphabet_assign ## count (__VA_ARGS__)
#define alphabet_assign_IMPL(count, ...) alphabet_assign_IMPL2(count, __VA_ARGS__)
#define alphabet_assign(...) alphabet_assign_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)


#define alphabet_args1 const decltype(a) &a
#define alphabet_args2 const decltype(a) &a, const decltype(b) &b
#define alphabet_args3 const decltype(a) &a, const decltype(b) &b, const decltype(c) &c
#define alphabet_args(n) alphabet_args ## n


#define alphabet_on_each(a,n) on_each(a,alphabet(n)) 

#define argcnt1 arg1
#define argcnt2 arg1, arg2
#define argcnt3 arg1, arg2, arg3
#define argcnt(n) argcnt ## n

#define argcnt_concat(n,x) CONCAT(x,argcnt(n))

#define argcnt_map1(x,e...) x(e *arg1)
#define argcnt_map2(x,e...) x(e *arg1), x(e *arg2)
#define argcnt_map3(x,e...) x(e *arg1), x(e *arg2), x(e *arg3)
#define argcnt_map_dref(x,n,e...) argcnt_map ## n (x,e)

#define argcnt_shared_ptr1 const std::shared_ptr<decltype(a)> arg1;
#define argcnt_shared_ptr2 const std::shared_ptr<decltype(a)> arg1; const std::shared_ptr<decltype(b)> arg2;
#define argcnt_shared_ptr3 const std::shared_ptr<decltype(a)> arg1; const std::shared_ptr<decltype(b)> arg2; const std::shared_ptr<decltype(c)> arg3;
#define argcnt_shared_ptr(n) argcnt_shared_ptr ## n

#define argcnt_on_each_alphabet1(x) arg1(x(a))
#define argcnt_on_each_alphabet2(x) arg1(x(a)), arg2(x(b))
#define argcnt_on_each_alphabet3(x) arg1(x(a)), arg2(x(b)), arg3(x(c))
#define argcnt_on_each_alphabet(n,x) argcnt_on_each_alphabet ## n(x)

template<typename... T>
auto handles_helper(const T&... t){
	auto ret = std::tuple_cat(::handles(*t)...);
	static_assert(std::tuple_size<decltype(ret)>::value > 0,
				  "Error: operation call with no handles");
	assert(std::tuple_size<decltype(ret)>::value > 0);
	return ret;
}

template<typename F>
void effect_map(const F&) {}

template<typename F, typename T1, typename... T>
void effect_map(const F& f, const T1 &t1, const T& ...t){
	f(t1);
	effect_map(f,t...);
}

template<typename Cache, typename Store, typename... T>
void run_strong_helper(Cache& c, Store &s, const T& ...t){
	effect_map([&](const auto &t){run_ast_strong(c,s,*t);},t...);
}

template<typename Cache, typename Store, typename... T>
void run_causal_helper(Cache& c, Store &s, const T& ...t){
	effect_map([&](const auto &t){run_ast_causal(c,s,*t);},t...);
}

#define do_op2(Name, n, _arg...)										\
	[&](){																\
		alphabet_assign(_arg);											\
		using FindUsage = FindUsages<alphabet_on_each(decltype, n)>;	\
		struct OperateImpl : public FindUsage,							\
							 public ConStatement<min_level_dref<alphabet_on_each(decltype,n)>::value >{ \
			using level = get_level<OperateImpl>;						\
			const int id = gensym();									\
			argcnt_shared_ptr(n)										\
				const std::string name = #Name;							\
			OperateImpl(alphabet_args(n) /*const decltype(a) &a*/ ):	\
				FindUsage(alphabet(n)),									\
				argcnt_on_each_alphabet(n,shared_copy)					\
				/*arg(shared_copy(a))*/{}								\
																		\
			auto handles() const {										\
				return handles_helper(argcnt(n));						\
																		\
			}															\
																		\
			auto strongCall(StrongCache& c CMA const StrongStore &s CMA std::true_type*) const { \
				auto ret = make_PreOp(id,Name(argcnt_map_dref(trans_op_arg,n,c,s,))) \
					(c,argcnt(n));										\
				c.emplace<decltype(ret)>(id,ret);						\
				return ret;												\
			}															\
																		\
			void strongCall(StrongCache& c CMA const StrongStore &s CMA std::false_type*) const { \
			}															\
			auto strongCall(StrongCache& c CMA  const StrongStore &s) const { \
				run_strong_helper(c,s,argcnt(n));						\
				choose_strong<level::value> choice{nullptr};			\
				return strongCall(c,s,choice);							\
			}															\
																		\
			auto causalCall(CausalCache& c CMA  const CausalStore &s) const { \
				run_causal_helper(c,s,argcnt(n));						\
				using R = decltype(make_PreOp(id,Name(argcnt_map_dref(trans_op_arg,n,c,s,))) \
								   (c,argcnt(n)));			\
				if(runs_with_strong(level::value)) return c.template get<R>(id); \
				else return make_PreOp(id,Name(argcnt_map_dref(trans_op_arg,n,c,s,))) \
									   (c,argcnt(n));		\
			}															\
		};																\
		OperateImpl r{alphabet(n)}; return r;				\
	} ()

#define do_op(name,...) STANDARD_BEGIN(do_op2(name,VA_NARGS(__VA_ARGS__), __VA_ARGS__))
