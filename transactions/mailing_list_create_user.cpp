#include "mailing_list_example.hpp"

namespace examples {
		user_hndl create_user(client<mailing_list_state>& ct, groups g){
			assert(g._ro);
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
		using connections = typename DECT(ct.trk)::connection_references;
		auto strong_connection = ct.get_relay<Level::strong>().lock();
		auto causal_connection = ct.get_relay<Level::causal>().lock();
		return txn.run_optimistic(ct.trk,&ct.dsm,
											 connections{ConnectionReference<Label<strong> >{*strong_connection},ConnectionReference<Label<causal> >{*causal_connection}},g);
	}
}
