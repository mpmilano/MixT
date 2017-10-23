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
	auto choice = (mutils::int_rand() % 40000)*2;
	auto &ret = cached_groups.at(choice);
	if (!ret) {
		ret.reset(new group{c.ss.template existingObject<typename group::users_lst>(nullptr,choice)});
	}
	return *ret;
}

user_hndl& mailing_list_state::pick_user(client<mailing_list_state>& c){
	auto choice = (mutils::int_rand() % 40000)*3;
	auto &ret = my_users.at(choice);
	if (!ret){
		auto hndl = c.sc.template existingObject<user>(nullptr,choice);
		ret.reset(new DECT(hndl){hndl});
	}
	return *ret;
}

void mailing_list_state::create_user(client<mailing_list_state>& c){
	auto ret = ::create_user(c,groups_linked_list);
	my_users.emplace_back(new DECT(ret){ret});
}

void mailing_list_state::create_group(client<mailing_list_state>& c){
	auto ret = create_global_group(c,groups_linked_list);
	cached_groups.emplace_back(new DECT(ret){ret});
}

template<typename SC, typename Ctxn>
auto create_user(std::size_t name, client<mailing_list_state>& c, SC &sc, Ctxn& ctxn){
	auto ret = sc.template newObject<user>(ctxn,name*3,user{sc.newObject(ctxn,name*3-2,inbox_str{
					sc.template newObject<message>(ctxn,name*3-1,"This is the head message. it will remain"),
						sc.template nullObject<inbox_str>()})});
	c.i.my_users.emplace_back(new DECT(ret){ret});
	return ret;
}

template<typename User, typename SS, typename Stxn>
auto create_group(std::size_t name, client<mailing_list_state>& c, const User &user, SS &ss, Stxn &stxn){
	auto ret = group{ss.newObject(stxn,name*2,typename group::users_lst{user,ss.template nullObject<typename group::users_lst>()})};
	c.i.cached_groups.emplace_back(new DECT(ret){ret});
	return ret;
}

template<typename User, typename SS, typename Stxn, typename Prev>
Prev create_and_append_group(std::size_t name, client<mailing_list_state>& c, const User &user, SS &ss, Stxn &stxn, const Prev& prev){
	return ss.newObject(stxn,name*2-1,groups_node{create_group(name,c,user, ss, stxn), prev });
}

mailing_list_state::mailing_list_state(client<mailing_list_state>& c)
#ifdef INITIALIZE_MAILING_LIST_EXAMPLE	
{
	auto &sc = c.sc;
	auto &ss = c.ss;
	auto _ctxn = sc.begin_transaction(whendebug("initial test setup"));
	auto _stxn = ss.begin_transaction(whendebug("initial test setup"));
	this->groups_linked_list = [&]{
		auto ctxn = dynamic_cast<typename DECT(sc)::SQLContext*>(_ctxn.get());
		auto stxn = dynamic_cast<typename DECT(ss)::SQLContext*>(_stxn.get());
		auto hd = ss.template nullObject<groups_node>();
		for (int i = 1u; i < 40000; ++i){
			try {
				hd = create_and_append_group(i,c,::create_user(i,c,sc,ctxn),ss,stxn,hd);
			}
			catch(const std::exception &e ){
				std::cerr << "We have failed to insert: " << i << " because " << e.what() << std::endl;
				throw e;
			}
		}
		return hd;
	}();
	_stxn->store_commit();
	_ctxn->store_commit();
}
#else
: groups_linked_list(c.ss.existingObject<groups_node>(nullptr,((40000-1)*2)-1) ),
		cached_groups(40000),my_users(40000){(void) c;}
#endif


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
	
