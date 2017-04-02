
#include "StoreRelay.hpp"
#include "SQLStore.hpp"

int main(int whendebug(argc), char** argv){
	using namespace myria;
	using namespace server;
	using namespace pgsql;
	using namespace mtl;
	using namespace std;
	using namespace mutils;
	using SQLInstanceManager = typename SQLStore<Label<STORE_LEVEL> >::SQLInstanceManager;
	using Relay = StoreRelay<SQLStore<Label<STORE_LEVEL> > >;
	using captive_store = typename Relay::captive_store;

	struct captive_sqlstore : public captive_store{
		SQLInstanceManager ss;
		DeserializationManager _dsm{{&ss}};
		SQLStore<Label<STORE_LEVEL > >& _store{ss.inst()};
		SQLStore<Label<STORE_LEVEL > >& store(){
			return _store;
		}
		DeserializationManager &dsm() {
			return _dsm;
		}

		captive_sqlstore(SQLConnectionPool<Label<STORE_LEVEL > >& pool)
			:ss{pool}{}
	};
	assert(argc >= 1);
	Relay relay{atoi(argv[1]), [pool = std::make_shared<SQLConnectionPool<Label<STORE_LEVEL > > >()]() mutable {
			return std::unique_ptr<captive_store>{
				new captive_sqlstore(*pool)}; }};

	
}
