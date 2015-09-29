#pragma once
#include "macro_utils.hpp"

#define free_expr3(T,a,e) ([&](){ using t = typename extract_type<decltype(a)>::type; struct Force_cexpr{static constexpr bool fun(const t &a) {ignore(e); return true;} }; \
								  assert(Force_cexpr::fun(*mke_p<t>())); \
								  return FreeExpr<T,decltype(a)>([&](const t &a){return e;}, a);}())

#define free_expr4(T,a,b,e) ([&](){ using ta = typename extract_type<decltype(a)>::type; \
									using tb = typename extract_type<decltype(b)>::type; \
									struct Force_cexpr{static constexpr bool fun(const ta &a, const tb &b){ignore(e);} return true;}; \
									assert(Force_cexpr::fun(*mke_p<ta>(),*mke_p<tb>())); \
									return FreeExpr<T,decltype(a),decltype(b)>([&](const ta &a, const tb &b){return e;},a,b)}())


#define free_expr_IMPL2(count, ...) free_expr ## count (__VA_ARGS__)
#define free_expr_IMPL(count, ...) free_expr_IMPL2(count, __VA_ARGS__)
#define free_expr(...) free_expr_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)

#define fld(a,b) free_expr(decltype(std::declval<run_result<decltype(a)> >().get().b), a, a.b)
#define msg(a,b,c...) free_expr(decltype(std::declval<run_result<decltype(a)> >().get().b(on_each_prn(std::declval<run_result<decltype, c, > >()))), a, c, a.b(c))
