#define STORE_LIST pgsql::SQLStore<pgsql::Level::STORE_LEVEL>
#include "server/StoreRelay.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "mtl/RemoteList.hpp"
#include "mailing_list_example.hpp"
#include "FinalHeader.hpp"

using namespace myria;
using namespace server;
using namespace pgsql;
using namespace mtl;
using namespace std;
using namespace mutils;
using Hndl = Handle<Label<STORE_LEVEL >, int, SupportedOperation<RegisteredOperations::Increment,void,SelfType> >;

template<typename l, typename r> struct transactions {
  using incr_trans = l;
  using read_trans = r;
	using mailing_list_add_new_user =
#include "mailing_list_add_new_user.cpp.precompiled"
		;
	using mailing_list_create_user =
#include "mailing_list_create_user.cpp.precompiled"
		;
	using mailing_list_download_inbox =
#include "mailing_list_download_inbox.cpp.precompiled"
		;
	using mailing_list_post_new_message =
#include "mailing_list_post_new_message.cpp.precompiled"
		;
	template<typename Store>
	using Relay = RelayForTransactions<Store, incr_trans, read_trans,
																		 mailing_list_add_new_user,
																		 mailing_list_create_user,
																		 mailing_list_download_inbox,
																		 mailing_list_post_new_message>;
};


constexpr auto build_transactions(const Hndl &hndl){
  constexpr auto incr_trans = TRANSACTION(remote x = hndl, x = x + 1)::WITH(hndl);
  constexpr auto read_trans = TRANSACTION(remote x = hndl, {})::WITH(hndl);
  return transactions<DECT(incr_trans), DECT(read_trans)>{};
}

int main(int whendebug(argc), char** argv){

	using both_transactions = DECT(build_transactions(std::declval<Hndl>()));
	
	using Relay = typename both_transactions::template Relay<SQLStore<Level::STORE_LEVEL> >;
	using captive_store = typename Relay::captive_store;

	struct captive_sqlstore : public captive_store{
		using SQLInstanceManager = typename SQLStore<Level::STORE_LEVEL >::SQLInstanceManager;
		SQLInstanceManager ss;
		DeserializationManager _dsm{{&ss}};
		SQLStore<Level::STORE_LEVEL>& _store{ss.inst()};
		SQLStore<Level::STORE_LEVEL>& store(){
			return _store;
		}
		DeserializationManager &dsm() {
			return _dsm;
		}

		captive_sqlstore(whenpool(LocalSQLConnectionPool& pool) whennopool(const std::string &host))
			:ss{whenpool(pool) whennopool(host)}{}
	};
	assert(argc >= 2);
	
	Relay relay{atoi(argv[1]), [whenpool(pool = std::make_shared<LocalSQLConnectionPool >())]() whenpool(mutable) {
			return std::unique_ptr<captive_store>{
				new captive_sqlstore(whenpool(*pool) whennopool("/run/postgresql"))}; }};
	relay.receiver.acceptor_fun();

	
}
