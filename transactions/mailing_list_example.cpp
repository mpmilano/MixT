#include "mailing_list_example.hpp"

#include "threaded_trial.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "test_utils.hpp"
#include "FinalHeader.hpp"
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

groups mailing_list_state::all_groups(client<mailing_list_state>&){
	struct die{}; throw die{};
}

group& mailing_list_state::pick_group(client<mailing_list_state>&){
	if (cached_groups.size() > 0) return cached_groups[mutils::int_rand() % cached_groups.size()];
	else { struct die{}; throw die{};}
}

user_hndl& mailing_list_state::pick_user(client<mailing_list_state>& c){
	if (my_users.size() > 0)
		return my_users[mutils::int_rand() % my_users.size()];
	else {
		my_users.emplace_back(create_user(c,all_groups(c)));
		return pick_user(c);
	}	
}

mailing_list_state::mailing_list_state(){
	struct die{}; throw die{};
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
				create_user(*this,i.all_groups(*this));
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
	
	int main(int argc, char **argv) {
		configuration_parameters params;
		assert(argc == 1 || argc == 16);
		if (argc == 1) {
			std::cin >> params;
		} else read_from_args(params,argv + 1);
		std::cout << params << std::endl;
		test<mailing_list_state> t1{params};
		t1.run_test();
		exit(0);
	}
