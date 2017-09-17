#include "mailing_list_example.hpp"

namespace examples {
		user_hndl create_user(ClientTrk& ct, groups& g){
		constexpr auto txn = TRANSACTION(
			var sample_user_hndl = (g->value).users->value.ensure(causal),
			var new_user = (*sample_user_hndl).ensure(causal),
			/*break off the inbox after the first message (which is the same across all inboxes)*/
			new_user.i->next = new_user.i->next.nulled().ensure(causal),
			return sample_user_hndl.new(new_user);
			)::WITH(g);
		return txn.run_local(ct,g);
	}
}
