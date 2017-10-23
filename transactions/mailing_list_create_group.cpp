#include "mailing_list_example.hpp"

namespace examples {
		group create_global_group(client<mailing_list_state>& ct, groups g){
#ifdef USE_PRECOMPILED
			constexpr
#include "mailing_list_create_group.cpp.precompiled"
				txn;
#else
		constexpr auto txn = TRANSACTION(
			var new_group = g->value,
			/* cut off the users list */
			new_group.users->next = new_group.users->next.nulled(),
			var new_grouplist_node = *g,
			new_grouplist_node.value = new_group,
			g->next = g.new(new_grouplist_node),
			return new_group
			)::WITH(g);

#endif
		using connections = typename DECT(ct.trk)::connection_references;
		auto strong_connection = ct.get_relay<Level::strong>().lock();
		auto causal_connection = ct.get_relay<Level::causal>().lock();
		return txn.run_optimistic(ct.trk,&ct.dsm,
											 connections{ConnectionReference<Label<strong> >{*strong_connection},ConnectionReference<Label<causal> >{*causal_connection}},g);
	}
}
