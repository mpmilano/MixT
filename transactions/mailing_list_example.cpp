#include "mailing_list_example.hpp"

using namespace examples;

int main(){

	ClientTrk ct;
	MidStore ms;
	BotStore bs;
	
	group g{ms.nullObject<typename group::users_lst>()};
	g.add_new_user(ct, bs.nullObject<user> ());
}
