#pragma once

#include <iostream>
#include "testing_store/TestingStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "testing_store/mid.hpp"
#include "mtl/RemoteList.hpp"
#include "mtl/split_printer.hpp"


namespace examples{
	using namespace myria;
using namespace testing_store;
using namespace tracker;

using MidStore = TestingStore<Label<mid> >;
using BotStore = TestingStore<Label<bottom> >;
using ClientTrk = ClientTracker<>;

using message = std::string;

using message_hndl = typename BotStore::template TestingHandle<message>;

using Inbox = typename RemoteList<message_hndl, BotStore::TestingHandle>::Hndl;

struct user {
	Inbox i;
	//boilerplate; must find a way to eliminate it.
	bool is_struct{ true };
	auto& field(MUTILS_STRING(i)) { return i; }
};

using user_hndl = typename BotStore::template TestingHandle<user>;


struct group {
	using users_lst = RemoteList<user_hndl,MidStore::TestingHandle>;
	typename users_lst::Hndl users;
	group(DECT(users) users):users(users){}
	
	void post_new_message(ClientTrk& ct, std::string message_contents);
	void add_new_user(ClientTrk& ct, user_hndl newbie);
	
};

}
namespace mutils {
	template<> struct typename_str<examples::user> {
		static std::string f(){ return "user"; }
	};
}
