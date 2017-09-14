#include "mailing_list_example.hpp"

namespace examples{
	void group::add_new_user(ClientTrk& ct, user_hndl newbie){
		TRANSACTION(
			var curr_user = users,
			while(curr_user.isValid() && curr_user->next.isValid()){
				curr_user = curr_user->next
			},
			remote mutable_alias = curr_user,
			var new_user_node = *curr_user,
			new_user_node.value = newbie,
			new_user_node.next = curr_user->next,
			mutable_alias.next = curr_user.new(new_user_node)
			)::RUN_LOCAL_WITH(ct,newbie,users);
	}
}
