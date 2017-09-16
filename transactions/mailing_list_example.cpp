#include "mailing_list_example.hpp"

using namespace examples;

int main(){

	ClientTrk ct;
	MidStore ms;
	BotStore bs;
	user u{bs.newObject(inbox_str{
				bs.template newObject<message>("hello"),
					bs.template nullObject<inbox_str>()})};

	auto ru = bs.newObject<user>(u);
	group g{ms.newObject(typename group::users_lst{ru,ms.template nullObject<typename group::users_lst>()}) };
	g.add_new_user(ct,ru);
	g.post_new_message(ct,"Hello There");
	for (const auto& str : u.download_inbox(ct)){
		std::cout << "got a message! " << str << std::endl;
	}
}
