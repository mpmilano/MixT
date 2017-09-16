#include "mailing_list_example.hpp"
#include "mtl/typecheck_printer.hpp"
#include "mtl/split_printer.hpp"
#include <iostream>

namespace examples{
	void group::add_new_user(ClientTrk& ct, user_hndl newbie){
		constexpr auto txn = 
		TRANSACTION(
			var curr_user = users.ensure(mid),
			while(curr_user.isValid() && curr_user->next.isValid()){
				curr_user = curr_user->next
			},
			var new_user_node = *curr_user,
			new_user_node.value = newbie.ensure(mid),
			new_user_node.next = curr_user->next,
			remote mutable_alias = curr_user,
			mutable_alias.next.ensure(mid) = curr_user.new(new_user_node)
			)::WITH(newbie,users);
		std::cout << txn << std::endl;
		txn.run_local(ct,newbie,users);
		
	}
}
