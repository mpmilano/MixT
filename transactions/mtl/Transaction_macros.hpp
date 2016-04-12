#pragma once
#include "macro_utils.hpp"
#include "FreeExpr_macros.hpp"

#define CMA ,

/*#define TRANS_CONS(x...) { auto prev2 = append(prev, curr);	\
  { auto prev = prev2; { x } } }
*/
#define TRANS_CONS(x...) {auto curr = x ;{ auto prev2 = append(prev,curr); { auto prev = prev2;
#define STANDARD_BEGIN(x...) (x);

#define END_TRANSACTION(cname) return mtl::Transaction<cname>{prev}; 

#include "trans_seq_generated.hpp"

#define TRANS_SEQ_IMPL2(count, ...) TRANS_SEQ ## count (__VA_ARGS__)
#define TRANS_SEQ_IMPL(count, ...) TRANS_SEQ_IMPL2(count, __VA_ARGS__)
#define TRANS_SEQ(...) TRANS_SEQ_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)


#define TRANSACTION(global_tracker,capture_name,args...) { using capture_t = std::decay_t<decltype(capture_name) >; const auto this_transaction = [capture_name = EnvironmentExpression<capture_t>{}](){using namespace mtl; using namespace mutils; mtl::TransactionBuilder<std::tuple<> > prev; TRANS_SEQ(args, END_TRANSACTION(capture_t))}(); this_transaction(global_tracker,&capture_name);}

//change these - mutable can use, just not dref.
#define let(x) [&]() { auto decl = MutDeclaration(#x); auto x = (mtl::MutAssigner(#x)

#define let_remote(x)  [&]() { auto decl = ImmutDeclaration(#x); auto x = (mtl::ImmutAssigner(#x)
#define IN(args...) ); (TRANS_SEQ(STANDARD_BEGIN(decl),args, STANDARD_BEGIN(mtl::end_var_scope()), return mtl::clobber(prev);));  }(); 

#define raw(x...) STANDARD_BEGIN(x)

#define IF(s...) STANDARD_BEGIN(mtl::make_if_begin((s)))
		
#define THEN(args...) , args, STANDARD_BEGIN(mtl::make_if_end())

#define WHILE(cond...) STANDARD_BEGIN(mtl::make_while_begin((cond)))

#define DO(things...) , things, STANDARD_BEGIN(mtl::make_while_end())

