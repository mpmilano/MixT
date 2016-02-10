TRANS_DIR= `pwd`/
IP_quad=`ifconfig | sed s/broadcast.*//g | tr " " "\n" | grep 128.84 | rev | sed s/'\..*'//g | rev`
storeList='pgsql::SQLStore<Level::strong>, pgsql::SQLStore<Level::causal>'
clangFlags= -I$(PWD) -I$(PWD)/mtl -I$(PWD)/mutils -I$(PWD)/testing -I$(PWD)/pgsql -I$(PWD)/tracker -g -stdlib=libc++ --std=c++14 -Wall -ferror-limit=1 -DNUM_CAUSAL_GROUPS="4" -DSTORE_LIST=$(storeList) -DMY_IP=\"$(MY_IP)\" -DCACHE_PORT=$(CACHE_PORT) -DMAX_THREADS=$(MAX_THREADS) -DDECLARED_OPERATIONS="template<Level l2> DECLARE_OPERATION(Increment, RemoteObject<l2,int>*) template<Level l2, typename T> DECLARE_OPERATION(Insert, RemoteObject<l2,std::set<T> >*, const T& ) "
linkFlags= -stdlib=libc++ --std=c++14 -lpqxx -lm -pthread 
object_files=FutureFreePool.o CooperativeCache.o Ends.o utils.o Tracker.o Ostreams.o SerializationSupport.o WhileBuilder.o ConExpr.o SQLStore.o TempBuilder.o TransactionBasics.o
export IP_quad
export TRANS_DIR
export clangFlags
export linkFlags

all: $(object_files) list_example.o
	clang++ $(object_files) list_example.o -o test $(linkFlags)

stress1: $(object_files) stress_test1.o
	clang++ $(object_files) stress_test.o -o stress1 $(linkFlags)

stress2: $(object_files) stress_test2.o
	clang++ $(object_files) stress_test.o -o stress2 $(linkFlags)

stress3:  $(object_files) stress_test3.o
	clang++ $(object_files) stress_test.o -o stress3 $(linkFlags)

vm: $(object_files) vm_main.o
	clang++ $(object_files) vm_main.o -o vm $(linkFlags)

cache_test: $(object_files) cache_interacter.o
	clang++ $(object_files) cache_interacter.o -o cache_test $(linkFlags)

cache_interacter.o:
	clang++ -c cache_interacter.cpp $(clangFlags)

chat: $(object_files) chat_example.o
	clang++ $(object_files) chat_example.o -o chat $(linkFlags)

sqlstore: sqlstore_clean all
	echo "done"

tracker: tracker_clean all
	echo "done"

stress_test1.o:
	clang++ -c stress_test.cpp -DTEST_MODE=1 $(clangFlags)
stress_test2.o:
	clang++ -c stress_test.cpp -DTEST_MODE=2 $(clangFlags)
stress_test3.o:
	clang++ -c stress_test.cpp -DTEST_MODE=3 $(clangFlags)
vm_main.o:
	clang++ -c vm_main.cpp -DconcurrencySetting=$(concurrencySetting) -D$(defineThis) -DIP_QUAD=$(IP_quad) -DSTRONG_REMOTE_IP=\"$(STRONG_REMOTE_IP)\" -DCAUSAL_GROUP=$(causalGroup) $(clangFlags)
sqlstore_clean:
	touch SQLStore.o; rm SQLStore.o
tracker_clean:
	rm Tracker.o;  true
list_example.o:
	clang++ -c list_example.cpp $(clangFlags)
chat_example.o:
	clang++ -c chat_example.cpp $(clangFlags)
utils.o:
	clang++ -c */utils.cpp $(clangFlags)
Ends.o:
	clang++ -c */Ends.cpp $(clangFlags)
Tracker.o:
	clang++ -c */Tracker.cpp $(clangFlags)
TransactionBasics.o:
	clang++ -c */TransactionBasics.cpp $(clangFlags)
Ostreams.o:
	clang++ -c Ostreams.cpp $(clangFlags)
SerializationSupport.o:
	clang++ -c */SerializationSupport.cpp $(clangFlags)
SQLStore.o:
	clang++ -c */SQLStore.cpp -DNUM_CAUSAL_MASTERS="2" -DCAUSAL_GROUP=$(causalGroup) $(clangFlags)
TempBuilder.o:
	clang++ -c */TempBuilder.cpp $(clangFlags)
ConExpr.o:
	clang++ -c */ConExpr.cpp $(clangFlags)
WhileBuilder.o:
	clang++ -c */WhileBuilder.cpp $(clangFlags)
CooperativeCache.o:
	clang++ -c */CooperativeCache.cpp $(clangFlags)
FutureFreePool.o:
	clang++ -c */FutureFreePool.cpp $(clangFlags)

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
