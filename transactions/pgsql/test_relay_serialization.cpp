#include "server/StoreRelay.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "mtl/split_printer.hpp"
#include <iostream>
#define STORE_LEVEL causal
#define STORE_LIST pgsql::SQLStore<pgsql::Level::STORE_LEVEL>
#include "FinalHeader.hpp"

using namespace myria;
using namespace server;
using namespace pgsql;
using namespace mtl;
using namespace std;
using namespace mutils;
using namespace runnable_transaction;
using SQLInstanceManager = typename SQLStore<Level::STORE_LEVEL >::SQLInstanceManager;
using Hndl = Handle<Label<STORE_LEVEL >, int, SupportedOperation<RegisteredOperations::Increment,void,SelfType> >;

void write_debug_file(int i, char* data, std::size_t size){
	std::string _i(std::to_string(i));
	std::ofstream out{std::string{"/tmp/deargod"} + _i};
	out.write(data,size);
}

int main(){

	SQLConnectionPool<Level::STORE_LEVEL > pool;
	SQLInstanceManager ss{pool};
	DeserializationManager dsm{{&ss}};
	constexpr auto name = 478446/2;
	Hndl hndl = ss.inst().template existingObject<int>(name);
	constexpr auto incr_trans = TRANSACTION(Hndl::label::int_id::value,let remote x = hndl in {x = x + 1})::WITH(hndl);
	constexpr auto read_trans = TRANSACTION(150 + Hndl::label::int_id::value,let remote x = hndl in {})::WITH(hndl);
	cout << incr_trans << endl;
	cout << read_trans << endl;
	using store = typename DECT(incr_trans)::all_store;

	for (int i = 0; i < 100; ++i){
		ClientRequestMessage<store> msg{whendebug(mutils::int_rand() ,) std::unique_ptr<store>(new store(initialize_store_values{},value_holder<Hndl,'h','n','d','l'>{hndl}))};
		common_interp<typename DECT(incr_trans)::template find_phase<Label<top> > >(*msg.store);
		auto size = bytes_size(msg);
		auto buf = std::unique_ptr<vector<char> >(new vector<char>(size,0));
		whendebug(auto tbsize = ) msg.to_bytes(buf->data());
		assert(size == tbsize);
		auto received_msg = ClientRequestMessage<store>::from_bytes(&dsm,buf->data());
	}

	{
		std::vector<int> test_this{932,3,41,34,134,34};
		char buf[bytes_size(test_this)];
		to_bytes(test_this,buf);
		assert(*from_bytes<DECT(test_this)>(nullptr,buf) == test_this);
	}

	for (int i = 0; i < 100; ++i){
		ServerReplyMessage<Name, store> msg{whendebug(mutils::int_rand() ,){}, std::unique_ptr<store>(new store(initialize_store_values{},value_holder<Hndl,'h','n','d','l'>{hndl}))};
		common_interp<typename DECT(incr_trans)::template find_phase<Label<top> > >(*msg.store);
		common_interp<typename DECT(incr_trans)::template find_phase<Label<causal> > >(*msg.store);
		//common_interp<typename DECT(incr_trans)::template find_phase<Label<bottom> > >(*msg.store);
		
		auto size = bytes_size(msg);
		auto buf = std::unique_ptr<vector<char> >(new vector<char>(size,0));
		whendebug(auto tbsize = ) msg.to_bytes(buf->data());
		assert(size == tbsize);
		write_debug_file(i,buf->data(),size);
		auto received_msg = ServerReplyMessage<Name, store>::from_bytes(&dsm,buf->data());
	}
}
