TRANS_DIR= `pwd`/
IP_quad=`ifconfig | sed s/broadcast.*//g | tr " " "\n" | grep 128.84 | rev | sed s/'\..*'//g | rev`
storeList='pgsql::SQLStore<Level::strong>, pgsql::SQLStore<Level::causal>, testing::TrackerTestingStore<Level::strong>, testing::TrackerTestingStore<Level::causal>'
CPPFLAGS= -I$(PWD) -I$(PWD)/mtl -I$(PWD)/myria-utils -I$(PWD)/mutils-networking -I$(PWD)/mutils -I$(PWD)/mutils-containers -I$(PWD)/mutils-serialization -I$(PWD)/mutils-tasks -I$(PWD)/testing -I$(PWD)/pgsql -I$(PWD)/tracker -g -p -stdlib=libc++ --std=c++14 -Wall -DNUM_CAUSAL_GROUPS="4" -DSTORE_LIST=$(storeList) -DMY_IP=\"$(MY_IP)\" -DMAX_THREADS=$(MAX_THREADS) -ferror-limit=1
LDFLAGS= -stdlib=libc++ --std=c++14 -lpqxx -lm -pthread
object_files=FutureFreePool.o CooperativeCache.o Ends.o utils.o Basics.o Tracker.o Ostreams.o SerializationSupport.o WhileBuilder.o ConExpr.o SQLStore.o TempBuilder.o TransactionBasics.o ServerSocket.o Socket.o
export IP_quad
export TRANS_DIR
export CPPFLAGS
export LDFLAGS

all: $(object_files) list_example.o
	clang++ $(object_files) list_example.o -o test $(LDFLAGS)

stress1: $(object_files) stress_test1.o
	clang++ $(object_files) stress_test.o -o stress1 $(LDFLAGS)

stress2: $(object_files) stress_test2.o
	clang++ $(object_files) stress_test.o -o stress2 $(LDFLAGS)

stress3:  $(object_files) stress_test3.o
	clang++ $(object_files) stress_test.o -o stress3 $(LDFLAGS)

vm: $(object_files) vm_main.o
	clang++ $(object_files) vm_main.o -o vm $(LDFLAGS)

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
	clang++ -c vm_main.cpp -DconcurrencySetting=$(concurrencySetting) -D$(defineThis) -DIP_QUAD=$(IP_quad) -DSTRONG_REMOTE_IP=\"$(STRONG_REMOTE_IP)\" -DCAUSAL_GROUP=$(causalGroup) $(CPPFLAGS)
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
Profiler.o:
	clang++ -c */Profiler.cpp $(CPPFLAGS)
GC_pointer.o:
	clang++ -c */GC_pointer.cpp $(CPPFLAGS)
Ends.o:
	clang++ -c */Ends.cpp $(CPPFLAGS)
Tracker.o:
	clang++ -c */Tracker.cpp $(CPPFLAGS) # && ln -s DisabledTracker.o Tracker.o
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
TempBuilder.o:
	clang++ -c */TempBuilder.cpp $(CPPFLAGS)
ConExpr.o:
	clang++ -c */ConExpr.cpp $(CPPFLAGS)
WhileBuilder.o:
	clang++ -c */WhileBuilder.cpp $(CPPFLAGS)
CooperativeCache.o:
	clang++ -c */CooperativeCache.cpp $(CPPFLAGS)
FutureFreePool.o:
	clang++ -c */FutureFreePool.cpp $(CPPFLAGS)

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
