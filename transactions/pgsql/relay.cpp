#include "server/StoreRelay.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "mtl/RemoteList.hpp"
#include "mailing_list_example.hpp"

using namespace myria;
using namespace server;
using namespace pgsql;
using namespace mtl;
using namespace std;
using namespace mutils;
using Hndl = Handle<Label<STORE_LEVEL >, int, SupportedOperation<RegisteredOperations::Increment,void,SelfType> >;
using Inherit = typename InheritGroup<>::template add_class_t<SQLStore<Level::STORE_LEVEL > >;
using RelayDSM = ::mutils::DeserializationManager<SQLStore<Level::STORE_LEVEL >, Inherit>;

template<typename l, typename r> struct transactions {
  using incr_trans = l;
  using read_trans = r;
	using StrongStore = examples::StrongStore;
	using CausalStore = examples::CausalStore;
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
	using mailing_list_create_group =
#include "mailing_list_create_group.cpp.precompiled"
		;
	template<typename Store>
	using Relay = RelayForTransactions<Store, RelayDSM, incr_trans, read_trans,
																		 mailing_list_add_new_user,
																		 mailing_list_create_user,
																		 mailing_list_download_inbox,
																		 mailing_list_post_new_message,
																		 mailing_list_create_group>;
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
		SQLStore<Level::STORE_LEVEL > ss;
		Inherit inherit;
		RelayDSM _dsm{&ss,&inherit};
		SQLStore<Level::STORE_LEVEL>& store(){
			return ss;
		}
		RelayDSM &dsm() {
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
