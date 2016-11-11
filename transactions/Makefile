TRANS_DIR= `pwd`/
IP_quad=`ifconfig | sed s/broadcast.*//g | tr " " "\n" | grep 128.84 | rev | sed s/'\..*'//g | rev`
storeList='pgsql::SQLStore<Level::strong>, pgsql::SQLStore<Level::causal>'
CPPFLAGS= -I$(PWD) -I$(PWD)/mtl -I$(PWD)/myria-utils -I$(PWD)/mutils-networking -I$(PWD)/mutils -I$(PWD)/mutils-containers -I$(PWD)/mutils-serialization -I$(PWD)/mutils-tasks -I$(PWD)/testing -I$(PWD)/pgsql -I$(PWD)/tracker -g -stdlib=libc++ --std=c++1z -Wall -DNUM_CAUSAL_GROUPS="4" -DSTORE_LIST=$(storeList) -DMY_IP=\"$(MY_IP)\" -DMAX_THREADS=$(MAX_THREADS) -DIP_QUAD=$(IP_quad) -DSTRONG_REMOTE_IP=\"$(STRONG_REMOTE_IP)\" -DCAUSAL_GROUP=$(causalGroup) -DCAUSAL_REMOTE_IP_1=\"$(CAUSAL_REMOTE_IP_1)\" -DCAUSAL_REMOTE_IP_2=\"$(CAUSAL_REMOTE_IP_2)\" $(extra_macro_defs) -ferror-limit=1 -Wall -Werror -Wextra
LDFLAGS= -stdlib=libc++ --std=c++1z -lpqxx -lm -pthread
object_files=FutureFreePool.o GlobalPool.o CooperativeCache.o Ends.o utils.o test_utils.o Basics.o Tracker.o Ostreams.o SerializationSupport.o  IfBuilder.o  WhileBuilder.o ConExpr.o SQLStore.o GSQLObject.o SQLConnection.o SQLTransaction.o SQL_internal_utils.o TempBuilder.o TransactionBasics.o ServerSocket.o Socket.o ClockManager.o abiutils.o batched_connection_common.o batched_connection_client.o batched_connection_server.o BlobUtils.o eventfd.o simple_rpc.o epoll.o

all: $(object_files) list_example.o
	clang++ $(object_files) list_example.o -o test $(LDFLAGS)

test_causal: $(object_files) 
	clang++ test_causal.cpp DisabledTracker.o SQLStore.o Ends.o utils.o Basics.o Ostreams.o SerializationSupport.o GSQLObject.o SQLConnection.o SQLTransaction.o SQL_internal_utils.o ServerSocket.o Socket.o batched_connection_client.o batched_connection_common.o BlobUtils.o -o test_causal $(CPPFLAGS) $(LDFLAGS)

stress1: $(object_files) strxess_test1.o
	clang++ $(object_files) stress_test.o -o stress1 $(LDFLAGS)

stress2: $(object_files) stress_test2.o
	clang++ $(object_files) stress_test.o -o stress2 $(LDFLAGS)

stress3:  $(object_files) stress_test3.o
	clang++ $(object_files) stress_test.o -o stress3 $(LDFLAGS)

vm: $(object_files) vm_main.o
	clang++ $(object_files) vm_main.o -o vm $(LDFLAGS)
groups: $(object_files) groups_test.o
	clang++ $(object_files) groups_test.o -o groups $(LDFLAGS)

cache_test: $(object_files) cache_interacter.o
	clang++ $(object_files) cache_interacter.o -o cache_test $(LDFLAGS)

cache_interacter.o:
	clang++ -c cache_interacter.cpp $(CPPFLAGS)

chat: $(object_files) chat_example.o
	clang++ $(object_files) chat_example.o -o chat $(LDFLAGS)

sqlstore: sqlstore_clean all
	echo "done"

tracker: tracker_clean all
	echo "done"

stress_test1.o:
	clang++ -c stress_test.cpp -DTEST_MODE=1 $(CPPFLAGS)
stress_test2.o:
	clang++ -c stress_test.cpp -DTEST_MODE=2 $(CPPFLAGS)
stress_test3.o:
	clang++ -c stress_test.cpp -DTEST_MODE=3 $(CPPFLAGS)
vm_main.o:
	clang++ -c vm_main.cpp -DNUM_CLIENTS=$(num_clients) -DCLIENT_RATE=$(client_rate)  -DINCREASE_BY=$(increase_by) -DINCREASE_DELAY=$(increase_delay) -DTEST_STOP_TIME=$(test_stop_time) $(CPPFLAGS)
groups_test.o:
	clang++ -c groups_test.cpp  $(CPPFLAGS)
sqlstore_clean:
	touch SQLStore.o; rm SQLStore.o
tracker_clean:
	rm Tracker.o;  true
list_example.o:
	clang++ -c list_example.cpp $(CPPFLAGS)
chat_example.o:
	clang++ -c chat_example.cpp $(CPPFLAGS)
Basics.o:
	clang++ -c Basics.cpp $(CPPFLAGS)
utils.o:
	clang++ -c */utils.cpp $(CPPFLAGS)
epoll.o:
	clang++ -c */epoll.cpp $(CPPFLAGS)
test_utils.o:
	clang++ -c test_utils.cpp $(CPPFLAGS)
Profiler.o:
	clang++ -c */Profiler.cpp $(CPPFLAGS)
GC_pointer.o:
	clang++ -c */GC_pointer.cpp $(CPPFLAGS)
eventfd.o:
	clang++ -c */eventfd.cpp $(CPPFLAGS)
Ends.o:
	clang++ -c */Ends.cpp $(CPPFLAGS)
#Tracker.o:
#	clang++ -c */Tracker.cpp $(CPPFLAGS) #&& ln -s DisabledTracker.o Tracker.o
Tracker.o:
	clang++ -c */DisabledTracker.cpp $(CPPFLAGS) && ln -s DisabledTracker.o Tracker.o
ClockManager.o:
	clang++ -c */ClockManager.cpp $(CPPFLAGS)
TransactionBasics.o:
	clang++ -c */TransactionBasics.cpp $(CPPFLAGS)
Ostreams.o:
	clang++ -c Ostreams.cpp $(CPPFLAGS)
SerializationSupport.o:
	clang++ -c */SerializationSupport.cpp $(CPPFLAGS)
ServerSocket.o:
	clang++ -c */ServerSocket.cpp $(CPPFLAGS)
Socket.o:
	clang++ -c */Socket.cpp $(CPPFLAGS)
SQLStore.o:
	clang++ -c */SQLStore.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
GSQLObject.o:
	clang++ -c */GSQLObject.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
SQLConnection.o:
	clang++ -c */SQLConnection.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
SQLTransaction.o:
	clang++ -c */SQLTransaction.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
SQL_internal_utils.o:
	clang++ -c */SQL_internal_utils.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
TempBuilder.o:
	clang++ -c */TempBuilder.cpp $(CPPFLAGS)
ConExpr.o:
	clang++ -c */ConExpr.cpp $(CPPFLAGS)
WhileBuilder.o:
	clang++ -c */WhileBuilder.cpp $(CPPFLAGS)
IfBuilder.o:
	clang++ -c */IfBuilder.cpp $(CPPFLAGS)
CooperativeCache.o:
	clang++ -c */CooperativeCache.cpp $(CPPFLAGS)
FutureFreePool.o:
	clang++ -c */FutureFreePool.cpp $(CPPFLAGS)
GlobalPool.o:
	clang++ -c */GlobalPool.cpp $(CPPFLAGS)
abiutils.o:
	clang++ -c */abiutils.cpp $(CPPFLAGS)
batched_connection_server.o:
	clang++ -c -O3 */batched_connection_server.cpp $(CPPFLAGS)

batched_connection_client.o:
	clang++ -c -O3 */batched_connection_client.cpp $(CPPFLAGS)

batched_connection_common.o:
	clang++ -c -O3 */batched_connection_common.cpp $(CPPFLAGS)

simple_rpc.o:
	clang++ -c */simple_rpc.cpp $(CPPFLAGS)
proxy_connection.o:
	clang++ -c */proxy_connection.cpp $(CPPFLAGS)
BlobUtils.o:
	clang++ -c */BlobUtils.cpp $(CPPFLAGS)


run: test
	PGHOST='127.0.0.1' PGPORT='5432' PGDATABASE='DataStore' PGUSER='research' PGPASSWORD='swimsand' ./test

debug: test
	PGHOST='127.0.0.1' PGPORT='5432' PGDATABASE='DataStore' PGUSER='research' PGPASSWORD='swimsand'  gdb ./test

testing: utils.o Tracker.o SerializationSupport.o WhileBuilder.o ConExpr.o SQLStore.o TempBuilder.o
	export TRANS_DIR=$(TRANS_DIR); $(MAKE) -C tests

sql_testing: utils.o Tracker.o SerializationSupport.o WhileBuilder.o ConExpr.o SQLStore.o TempBuilder.o
	export TRANS_DIR=$(TRANS_DIR); cd tests && $(MAKE) sql


clean:
	rm *.o; rm chat; rm test; rm stress1; rm stress2; rm stress3; rm vm
