#include "Handle.hpp"
#include "pgsql/SQLStore.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include <iostream>

using namespace myria;
using namespace mtl;
using namespace pgsql;
using namespace mutils;
using namespace tracker;
using namespace mtl;

int main(){
	SQLStore<Level::strong> ss_store{""};
	SQLStore_impl &ss_impl = ss_store;
	SQLStore<Level::causal> sc_store{""};
	SQLStore_impl &sc_impl = sc_store;
	std::cout << "stores constructed" << std::endl;
	using Inherit = typename InheritGroup<>::template add_class_t<DECT(ss_store)>::template add_class_t<DECT(sc_store)>;
	Inherit i;
	DeserializationManager<DECT(ss_store), DECT(sc_store), Inherit> dsm{&ss_store,&sc_store,&i};
	InheritGroup<>::template add_class_t<DECT(sc_store)> dig;
	DeserializationManager<DECT(ss_store), DECT(sc_store), DECT(dig) > degenerate{&ss_store,&sc_store,&dig};
	typename DECT(ss_store)::SQLContext stxn{ss_impl.begin_transaction("debug")};
	typename DECT(sc_store)::SQLContext ctxn{sc_impl.begin_transaction("debug")};
	auto hndl = ss_store.template newObject(&stxn , 5);
	char buf[hndl.bytes_size()];
	hndl.to_bytes(buf);
	auto dhndl = *DECT(hndl)::from_bytes(&dsm,buf);
	ClientTracker<> ct;
	stxn.store_commit();
	ctxn.store_commit();
	stxn.~SQLContext();
	ctxn.~SQLContext();
	int b = TRANSACTION(return (*hndl) + (*dhndl))::RUN_LOCAL_WITH(ct,&dsm,hndl,dhndl);
	assert(b == 10);
	auto degenerate_handle = *DECT(hndl)::from_bytes(&degenerate, buf);
	char dgnbuf[degenerate_handle.bytes_size()];
	degenerate_handle.to_bytes(dgnbuf);
	auto rhndl = *DECT(hndl)::from_bytes(&dsm,dgnbuf);
	int b2 = TRANSACTION(return (*hndl) + (*rhndl))::RUN_LOCAL_WITH(ct,&dsm,hndl,rhndl);
	assert(b2 == 10);
}
