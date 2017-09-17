#pragma once

#include <iostream>
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/RemoteList.hpp"
#include "mtl/split_printer.hpp"
#include "FinalHeader.hpp"
#include "threaded_trial.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "test_utils.hpp"
#include "FinalHeader.hpp"
#include "configuration_params.hpp"

namespace examples{
	using namespace myria;
	using namespace pgsql;
	using namespace tracker;

	using StrongStore = SQLStore<Level::strong>;
	using CausalStore = SQLStore<Level::causal>;
	using ClientTrk = ClientTracker<>;

using message = std::string;

using message_hndl = typename CausalStore::template SQLHandle<message>;

using inbox_str = RemoteList<message_hndl, CausalStore::SQLHandle>;
using Inbox = typename inbox_str::Hndl;

struct user {
	Inbox i;
	
	//boilerplate; must find a way to eliminate it.
	bool is_struct{ true };
	auto& field(MUTILS_STRING(i)) { return i; }
};

	using user_hndl = typename CausalStore::template SQLHandle<user>;
	std::list<message> download_inbox(ClientTrk& ct, user_hndl h);


struct group {
	using users_lst = RemoteList<user_hndl,StrongStore::SQLHandle>;
	typename users_lst::Hndl users;
	
	void post_new_message(ClientTrk& ct, std::string message_contents);
	void add_new_user(ClientTrk& ct, user_hndl newbie);

	//boilerplate; must find a way to eliminate it.
	bool is_struct{ true };
	auto& field(MUTILS_STRING(users)) { return users; }
	
};

	using groups = typename RemoteList<group, StrongStore::SQLHandle>::Hndl;
	user_hndl create_user(ClientTrk& ct, groups& g);

}
namespace mutils {
	template<> struct typename_str<examples::user> {
		static std::string f(){ return "user"; }
	};
	template<> struct typename_str<examples::group> {
		static std::string f(){ return "group"; }
	};
}
