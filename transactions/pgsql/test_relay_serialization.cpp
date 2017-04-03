#include "StoreRelay.hpp"
#include "SQLStore.hpp"
#include "transaction.hpp"
#include "transaction_macros.hpp"
#define STORE_LEVEL pgsql::causal
#define STORE_LIST pgsql::SQLStore<Label<STORE_LEVEL> >
#include "FinalHeader.hpp"

using namespace myria;
using namespace server;
using namespace pgsql;
using namespace mtl;
using namespace std;
using namespace mutils;
using namespace runnable_transaction;
using SQLInstanceManager = typename SQLStore<Label<STORE_LEVEL> >::SQLInstanceManager;
using Hndl = Handle<Label<STORE_LEVEL >, int, SupportedOperation<RegisteredOperations::Increment,void,SelfType> >;

int main(){

	Hndl hndl;
	constexpr auto incr_trans = TRANSACTION(Hndl::label::int_id::value,let remote x = hndl in {x = x + 1})::WITH(hndl);
	//constexpr auto read_trans = TRANSACTION(150 + Hndl::label::int_id::value,let remote x = hndl in {})::WITH(hndl);
	SQLConnectionPool<Label<STORE_LEVEL > > pool;
	SQLInstanceManager ss{pool};
	DeserializationManager dsm{{&ss}};

	using store = typename DECT(incr_trans)::all_store;

	for (int i = 0; i < 100; ++i){
		ServerReplyMessage<Name, store> msg{{}, std::unique_ptr<store>(new store(initialize_store_values{},value_holder<Hndl,'h','n','d','l'>{hndl}))};
		
		auto size = bytes_size(msg);
		auto buf = std::unique_ptr<vector<char> >(new vector<char>(size,0));
		whendebug(auto tbsize = ) msg.to_bytes(buf->data());
		assert(size == tbsize);
		auto received_msg = ServerReplyMessage<Name, store>::from_bytes(&dsm,buf->data());
	}

	for (int i = 0; i < 100; ++i){
		ClientRequestMessage<store> msg{std::unique_ptr<store>(new store(initialize_store_values{},value_holder<Hndl,'h','n','d','l'>{hndl}))};
		
		auto size = bytes_size(msg);
		auto buf = std::unique_ptr<vector<char> >(new vector<char>(size,0));
		whendebug(auto tbsize = ) msg.to_bytes(buf->data());
		assert(size == tbsize);
		auto received_msg = ClientRequestMessage<store>::from_bytes(&dsm,buf->data());
	}
}
