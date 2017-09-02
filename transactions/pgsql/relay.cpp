
#include "StoreRelay.hpp"
#include "SQLStore.hpp"
#include "transaction.hpp"
#include "transaction_macros.hpp"
#define STORE_LIST pgsql::SQLStore<pgsql::Level::STORE_LEVEL>
#include "FinalHeader.hpp"

using namespace myria;
using namespace server;
using namespace pgsql;
using namespace mtl;
using namespace std;
using namespace mutils;
using SQLInstanceManager = typename SQLStore<Level::STORE_LEVEL >::SQLInstanceManager;
using Hndl = Handle<Label<STORE_LEVEL >, int, SupportedOperation<RegisteredOperations::Increment,void,SelfType> >;

template<typename l, typename r> struct transactions {
  using incr_trans = l;
  using read_trans = r;
};


constexpr auto build_transactions(const Hndl &hndl){
  constexpr auto incr_trans = TRANSACTION(remote x = hndl, x = x + 1)::WITH(hndl);
  constexpr auto read_trans = TRANSACTION(remote x = hndl, {})::WITH(hndl);
  return transactions<DECT(incr_trans), DECT(read_trans)>{};
}

int main(int whendebug(argc), char** argv){

	using both_transactions = DECT(build_transactions(std::declval<Hndl>()));
	typename both_transactions::incr_trans incr_trans;
	typename both_transactions::read_trans read_trans;
	
	using Relay = RelayForTransactions<SQLStore<Level::STORE_LEVEL>, DECT(incr_trans), DECT(read_trans) >;
	using captive_store = typename Relay::captive_store;

	struct captive_sqlstore : public captive_store{
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
