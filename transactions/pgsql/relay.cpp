
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

int main(int whendebug(argc), char** argv){

	Hndl hndl;
	constexpr auto incr_trans = TRANSACTION(Hndl::label::int_id::value,let remote x = hndl in {x = x + 1})::WITH(hndl);
	constexpr auto read_trans = TRANSACTION(150 + Hndl::label::int_id::value,let remote x = hndl in {})::WITH(hndl);
	
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

		captive_sqlstore(LocalSQLConnectionPool& pool)
			:ss{pool}{}
	};
	assert(argc >= 2);
	
	Relay relay{atoi(argv[1]), [pool = std::make_shared<LocalSQLConnectionPool >()]() mutable {
			return std::unique_ptr<captive_store>{
				new captive_sqlstore(*pool)}; }};
	relay.receiver.acceptor_fun();

	
}
