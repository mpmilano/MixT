#include "mailing_list_example.hpp"
#include "mtl/typecheck_printer.hpp"
#include "mtl/split_printer.hpp"
#include <iostream>

namespace examples{
	void group::add_new_user(client<mailing_list_state>& ct, user_hndl newbie){
#ifdef USE_PRECOMPILED
		constexpr
#include "mailing_list_add_new_user.cpp.precompiled"
			txn;
#else
		constexpr auto txn = 
		TRANSACTION(
			var curr_user = users,
			while(curr_user.isValid() && curr_user->next.isValid()){
				curr_user = curr_user->next
			},
			var new_user_node = *curr_user,
			new_user_node.value = newbie,
			new_user_node.next = curr_user->next,
			remote mutable_alias = curr_user,
			mutable_alias.next = curr_user.new(new_user_node)
			)::WITH(newbie,users);
#endif
		std::cout << txn << std::endl;
		txn.run_local(ct.trk,newbie,users);
	}
}
