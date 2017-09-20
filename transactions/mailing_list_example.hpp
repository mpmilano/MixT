#pragma once

#include <iostream>
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/RemoteList.hpp"
#include "mtl/split_printer.hpp"
#include "threaded_trial.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "test_utils.hpp"
#include "configuration_params.hpp"
#include "test_client.hpp"

namespace examples{
	using namespace myria;
	using namespace pgsql;
	using namespace tracker;

	struct mailing_list_state;
	
	using StrongStore = SQLStore<Level::strong>;
	using CausalStore = SQLStore<Level::causal>;

using message = std::string;

using message_hndl = typename CausalStore::template SQLHandle<message>;

using inbox_str = RemoteList<message_hndl, CausalStore::SQLHandle>;
using Inbox = typename inbox_str::Hndl;

	struct user : public mutils::ByteRepresentable{
	Inbox i;
		DEFAULT_SERIALIZATION_SUPPORT(user,i);
		user(DECT(i) i):i(i){}
	//boilerplate; must find a way to eliminate it.
	bool is_struct{ true };
	auto& field(MUTILS_STRING(i)) { return i; }
};

	using user_hndl = typename CausalStore::template SQLHandle<user>;
	std::list<message> download_inbox(client<mailing_list_state>& ct, user_hndl h);


	struct group : public mutils::ByteRepresentable{
		using users_lst = RemoteList<user_hndl,StrongStore::SQLHandle>;
		
		typename users_lst::Hndl users;
		group(DECT(users) users):users(users){}

		DEFAULT_SERIALIZATION_SUPPORT(group,users);
		
		void post_new_message(client<mailing_list_state>& ct, std::string message_contents);
		void add_new_user(client<mailing_list_state>& ct, user_hndl newbie);
		
		//boilerplate; must find a way to eliminate it.
		bool is_struct{ true };
		auto& field(MUTILS_STRING(users)) { return users; }
	
};

	using groups_node = RemoteList<group, StrongStore::SQLHandle>;
	using groups = typename groups_node::Hndl;
	user_hndl create_user(client<mailing_list_state>& ct, groups g);
	group create_global_group(client<mailing_list_state>& ct, groups g);

	struct mailing_list_state{
		static groups all_groups(client<mailing_list_state>&);
		std::vector<group> cached_groups;
		std::vector<user_hndl> my_users;
		group& pick_group(client<mailing_list_state>&);
		user_hndl& pick_user(client<mailing_list_state>&);
		void create_user(client<mailing_list_state>&);
		void create_group(client<mailing_list_state>&);
		mailing_list_state();
	};

}
namespace mutils {
	template<> struct typename_str<examples::user> {
		static std::string f(){ return "user"; }
	};
	template<> struct typename_str<examples::group> {
		static std::string f(){ return "group"; }
	};
}
