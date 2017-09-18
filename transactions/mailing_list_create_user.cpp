#include "mailing_list_example.hpp"

namespace examples {
		user_hndl create_user(client<mailing_list_state>& ct, groups g){
#ifdef USE_PRECOMPILED
			constexpr
#include "mailing_list_create_user.cpp.precompiled"
				txn;
#else
		constexpr auto txn = TRANSACTION(
			var sample_user_hndl = (g->value).users->value.ensure(causal),
			var new_user = (*sample_user_hndl).ensure(causal),
			/*break off the inbox after the first message (which is the same across all inboxes)*/
			new_user.i->next = new_user.i->next.nulled().ensure(causal),
			return sample_user_hndl.new(new_user);
			)::WITH(g);
#endif
		return txn.run_local(ct.trk,g);
	}
}
