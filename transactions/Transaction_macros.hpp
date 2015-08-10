#include "macro_utils.hpp"
#include "Operate_macros.hpp"

#define CMA ,

/*#define TRANS_CONS(x...) { auto prev2 = append(prev, curr);	\
	{ auto prev = prev2; { x } } }
*/
#define TRANS_CONS(x...) {auto curr = x { auto prev2 = append(prev,curr); { auto prev = prev2;
#define STANDARD_BEGIN(x...) (x);

#define END_TRANSACTION Transaction ____transaction(prev); std::cout << ____transaction << std::endl; ____transaction();

#include "trans_seq_generated.hpp"

#define TRANS_SEQ_IMPL2(count, ...) TRANS_SEQ ## count (__VA_ARGS__)
#define TRANS_SEQ_IMPL(count, ...) TRANS_SEQ_IMPL2(count, __VA_ARGS__)
#define TRANS_SEQ(...) TRANS_SEQ_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)


#define TRANSACTION(args...) { TransactionBuilder<std::tuple<>, std::tuple<> > prev; TRANS_SEQ(args, END_TRANSACTION)}


#define let_mutable(x) [&]() { auto x = (MutAssigner(#x)

#define let_ifValid(x)  [&]() { auto x = (ImmutAssigner(#x)
#define IN(args...) ); (TRANS_SEQ(args, return clobber(prev);));  }(); 

#define raw(x...) STANDARD_BEGIN(x)

#define IF(s...) STANDARD_BEGIN(make_if_begin((s)))
		
#define THEN(args...) , args, STANDARD_BEGIN(make_if_end())

#define WHILE(cond...) STANDARD_BEGIN(make_while_begin((cond)))

#define DO(things...) , things, STANDARD_BEGIN(make_while_end())
