#pragma once
#include "macro_utils.hpp"

#define free_expr3(T,a,e) (FreeExpr<T,decltype(a)>([&](const typename extract_type<decltype(a)>::type &a){return e;},a))
#define free_expr4(T,a,b,e) (FreeExpr<T,decltype(a),decltype(b)>([&](const typename extract_type<decltype(a)>::type &a, \
																	 const typename extract_type<decltype(b)>::type &b){return e;},a,b))


#define free_expr_IMPL2(count, ...) free_expr ## count (__VA_ARGS__)
#define free_expr_IMPL(count, ...) free_expr_IMPL2(count, __VA_ARGS__)
#define free_expr(...) free_expr_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)

#define msg(a,b) free_expr(decltype(std::declval<run_result<decltype(a)> >().get().b), a, a.b)

#define dref(a) free_expr(typename extract_type<decltype(a)>::type,a,a)
