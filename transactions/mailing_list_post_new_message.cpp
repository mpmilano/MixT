#include "mailing_list_example.hpp"

namespace examples{
	void group::post_new_message(ClientTrk& ct, std::string message_contents){
		TRANSACTION(
			var curr_user = users,
			/*iterate through the users list*/
			while (curr_user.isValid()){
				/*will hold the end of the user messages list*/
				var user_msgs_tl = curr_user->value->i,
				/* advance to the end of messages here */
				while (user_msgs_tl.isValid() && user_msgs_tl->next.isValid() ){
					user_msgs_tl = user_msgs_tl->next
				},
				/*make a copy to base the new node on*/
				var new_msg_node = *user_msgs_tl,
				new_msg_node.value = new_msg_node.value.new(message_contents),
				new_msg_node.next = user_msgs_tl->next,
				/*deref the current tail and replace it with the new node*/
				remote derefd_user_msgs_tl = user_msgs_tl,
				derefd_user_msgs_tl.next = user_msgs_tl.new(new_msg_node),
				/*advance the while-loop*/
				curr_user = curr_user->next
			}
			)::RUN_LOCAL_WITH(ct,message_contents,users);
	}
}
