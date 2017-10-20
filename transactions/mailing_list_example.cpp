#include "mailing_list_example.hpp"

#include "threaded_trial.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "test_utils.hpp"
#include "configuration_params.hpp"

using namespace myria;
using namespace server;
using namespace pgsql;
using namespace mtl;
using namespace std;
using namespace chrono;
using namespace mutils;
using namespace examples;



using namespace examples;

group& mailing_list_state::pick_group(client<mailing_list_state>& c){
	if (cached_groups.size() > 0) return cached_groups[mutils::int_rand() % cached_groups.size()];
	else { create_group(c); return pick_group(c);}
}

user_hndl& mailing_list_state::pick_user(client<mailing_list_state>& c){
	if (my_users.size() > 0)
		return my_users[mutils::int_rand() % my_users.size()];
	else {
		create_user(c);
		return pick_user(c);
	}	
}

void mailing_list_state::create_user(client<mailing_list_state>& c){
	my_users.push_back(::create_user(c,groups_linked_list));
}

void mailing_list_state::create_group(client<mailing_list_state>& c){
	cached_groups.push_back(create_global_group(c,groups_linked_list));
}

template<typename SC, typename Ctxn>
auto create_user(client<mailing_list_state>& c, SC &sc, Ctxn& ctxn){
	auto ret = sc.template newObject<user>(ctxn,user{sc.newObject(ctxn,inbox_str{
					sc.template newObject<message>(ctxn,"This is the head message. it will remain"),
						sc.template nullObject<inbox_str>()})});
	c.i.my_users.emplace_back(ret);
	return ret;
}

template<typename User, typename SS, typename Stxn>
auto create_group(client<mailing_list_state>& c, const User &user, SS &ss, Stxn &stxn){
	auto ret = group{ss.newObject(stxn,typename group::users_lst{user,ss.template nullObject<typename group::users_lst>()})};
	c.i.cached_groups.emplace_back(ret);
	return ret;
}

template<typename User, typename SS, typename Stxn, typename Prev>
Prev create_and_append_group(client<mailing_list_state>& c, const User &user, SS &ss, Stxn &stxn, const Prev& prev){
	return ss.newObject(stxn,groups_node{create_group(c,user, ss, stxn), prev });
}

mailing_list_state::mailing_list_state(client<mailing_list_state>& c){
	auto &sc = c.sc;
	auto &ss = c.ss;
	auto _ctxn = sc.begin_transaction(whendebug("initial test setup"));
	auto _stxn = ss.begin_transaction(whendebug("initial test setup"));
	this->groups_linked_list = [&]{
		auto ctxn = dynamic_cast<typename DECT(sc)::SQLContext*>(_ctxn.get());
		auto stxn = dynamic_cast<typename DECT(ss)::SQLContext*>(_stxn.get());
		auto hd = ss.template nullObject<groups_node>();
		for (int i = 0u; i < 40000; ++i){
			hd = create_and_append_group(c,::create_user(c,sc,ctxn),ss,stxn,hd);
		}
		return hd;
	}();
	_stxn->store_commit();
	_ctxn->store_commit();
}

	enum class action_choice{
		post_new_message, add_new_user,
			create_user, download_inbox
	};

	action_choice choose_action(){
		auto choice = mutils::better_rand();
		assert(choice > 0);
		assert(choice < 1);
		if (choice < .25){
			return action_choice::post_new_message;
		} else if (choice < .5){
			return action_choice::add_new_user;
		} else if (choice < .75) {
			return action_choice::create_user;
		}
		else {
			return action_choice::download_inbox;
		}
	}

	template<>
	std::unique_ptr<run_result> & client<mailing_list_state>::client_action(std::unique_ptr<run_result> &result) {
		//four options: post message, join group, create user, download_inbox
		auto choice = choose_action();
		if (result){
			if (choice == action_choice::download_inbox) result->l = Level::causal;
			else result->l = Level::strong;
		}
		if (result){
			if (choice == action_choice::download_inbox) result->is_write = true;
			else result->is_write = false;
		}

		try {
			switch(choice){
			case action_choice::post_new_message:
				i.pick_group(*this).post_new_message(*this, "This is the only message");
				break;
			case action_choice::add_new_user:
				i.pick_group(*this).add_new_user(*this,i.pick_user(*this));
				break;
			case action_choice::create_user:
				i.create_user(*this);
				break;
			case action_choice::download_inbox:
				download_inbox(*this,i.pick_user(*this));
				break;
			}
		} catch (const SerializationFailure &sf){
			if (result){
				result->is_abort = true;
				result->abort_string = sf.what();
			}
		}
		if (result) result->stop_time = high_resolution_clock::now();
		return result;
	}
	
