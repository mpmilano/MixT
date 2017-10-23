#include "mailing_list_example.hpp"
#include "mtl/typecheck_printer.hpp"

namespace examples{
	void group::post_new_message(client<mailing_list_state>& ct, std::string message_contents){
		assert(users._ro);
#ifdef USE_PRECOMPILED
		constexpr 
#include "mailing_list_post_new_message.cpp.precompiled"
			txn;
#else
		using pre_txn = TRANSACTION(
			var index = users,
			/*iterate through the users list*/
			while (index.isValid()) {
				var curr_user = index,
				/*advance the while-loop*/
				index = index->next,
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
				derefd_user_msgs_tl.next = user_msgs_tl.new(new_msg_node)
			}
			);
		auto txn = pre_txn::WITH(message_contents,users);
#endif
		using namespace myria::mtl::typecheck_phase;
		using namespace myria::mtl::split_phase;
		using connections = typename DECT(ct.trk)::connection_references;
		auto strong_connection = ct.get_relay<Level::strong>().lock();
		auto causal_connection = ct.get_relay<Level::causal>().lock();
		txn.run_optimistic(ct.trk,&ct.dsm,
											 connections{ConnectionReference<Label<strong> >{*strong_connection},ConnectionReference<Label<causal> >{*causal_connection}},
											 message_contents,users);
	}
}
