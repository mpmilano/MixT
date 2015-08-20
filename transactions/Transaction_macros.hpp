#include "macro_utils.hpp"
#include "Operate_macros.hpp"

#define CMA ,

/*#define TRANS_CONS(x...) { auto prev2 = append(prev, curr);	\
	{ auto prev = prev2; { x } } }
*/
#define TRANS_CONS(x...) {static auto curr = x ;{ static auto prev2 = append(prev,curr); { static auto prev = prev2;
#define STANDARD_BEGIN(x...) (x);

#define END_TRANSACTION Transaction ____transaction(prev); std::cout << ____transaction << std::endl << "all done printing" << std::endl; ____transaction();

#include "trans_seq_generated.hpp"

#define TRANS_SEQ_IMPL2(count, ...) TRANS_SEQ ## count (__VA_ARGS__)
#define TRANS_SEQ_IMPL(count, ...) TRANS_SEQ_IMPL2(count, __VA_ARGS__)
#define TRANS_SEQ(...) TRANS_SEQ_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)


#define TRANSACTION(args...) { TransactionBuilder<std::tuple<> > prev; TRANS_SEQ(args, END_TRANSACTION)}


#define let_mutable(x) [&]() { static auto decl = MutDeclaration(#x); static auto x = (MutAssigner(#x)

#define let_ifValid(x)  [&]() { static auto decl = ImmutDeclaration(#x); static auto x = (ImmutAssigner(#x)
#define IN(args...) ); (TRANS_SEQ(STANDARD_BEGIN(decl),args, STANDARD_BEGIN(end_var_scope()), return clobber(prev);));  }(); 

#define raw(x...) STANDARD_BEGIN(x)

#define IF(s...) STANDARD_BEGIN(make_if_begin((s)))
		
#define THEN(args...) , args, STANDARD_BEGIN(make_if_end())

#define WHILE(cond...) STANDARD_BEGIN(make_while_begin((cond)))

#define DO(things...) , things, STANDARD_BEGIN(make_while_end())
