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
		mtl::PhaseContext<Label<mid> > ctx;
	ctx.store_context(ms,"asserting stuff");
	assert(g.users.isValid(&ctx));
	assert(!g.users.get(&ctx)->next.isValid(&ctx));
	g.add_new_user(ct,ru);
	assert(g.users.isValid(&ctx));
	assert(g.users.get(&ctx)->next.isValid(&ctx));
	assert(!g.users.get(&ctx)->next.get(&ctx)->next.isValid(&ctx));
	g.post_new_message(ct,"Hello There");
	assert(g.users.isValid(&ctx));
	assert(g.users.get(&ctx)->next.isValid(&ctx));
	assert(!g.users.get(&ctx)->next.get(&ctx)->next.isValid(&ctx));
	for (const auto& str : u.download_inbox(ct)){
		std::cout << "got a message! " << str << std::endl;
	}
}
