TRANS_DIR= `pwd`/
storeList='SQLStore<Level::strong>, SQLStore<Level::causal>'
clangFlags= -Xclang -fcolor-diagnostics -g -stdlib=libc++ --std=c++14 -Wall -ferror-limit=1 -DNUM_CAUSAL_GROUPS="4" -DSTORE_LIST=$(storeList) -DDECLARED_OPERATIONS="template<Level l2> DECLARE_OPERATION(Increment, RemoteObject<l2,int>*) template<Level l2, typename T> DECLARE_OPERATION(Insert, RemoteObject<l2,std::set<T> >*, const T& ) "
linkFlags= -stdlib=libc++ --std=c++14 -lpqxx
export TRANS_DIR
export clangFlags
export linkFlags

all: Ends.o utils.o Tracker.o Ostreams.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o new_test.o
	clang++ utils.o Ends.o Tracker.o Ostreams.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o new_test.o -o test $(linkFlags)

stress1: Ends.o utils.o Tracker.o Ostreams.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o stress_test1.o
	clang++ utils.o Ends.o Tracker.o Ostreams.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o stress_test.o -o stress1 $(linkFlags)

stress2: Ends.o utils.o Tracker.o Ostreams.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o stress_test2.o
	clang++ utils.o Ends.o Tracker.o Ostreams.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o stress_test.o -o stress2 $(linkFlags)

stress3: Ends.o utils.o Tracker.o Ostreams.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o stress_test3.o
	clang++ utils.o Ends.o Tracker.o Ostreams.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o stress_test.o -o stress3 $(linkFlags)

chat: Ends.o utils.o Ostreams.o Tracker.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o chat_example.o
	clang++ utils.o Ends.o Ostreams.o Tracker.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o chat_example.o -o chat $(linkFlags)

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
sqlstore_clean:
	touch SQLStore.o; rm SQLStore.o
tracker_clean:
	rm Tracker.o;  true
new_test.o:
	clang++ -c new_test.cpp $(clangFlags)
chat_example.o:
	clang++ -c chat_example.cpp $(clangFlags)
utils.o:
	clang++ -c utils.cpp $(clangFlags)
Ends.o:
	clang++ -c Ends.cpp $(clangFlags)
Tracker.o:
	clang++ -c Tracker.cpp $(clangFlags)
Ostreams.o:
	clang++ -c Ostreams.cpp $(clangFlags)
SerializationSupport.o:
	clang++ -c SerializationSupport.cpp $(clangFlags)
SQLStore.o:
	clang++ -c SQLStore.cpp -DNUM_CAUSAL_MASTERS="2" $(clangFlags)
Context.o:
	clang++ -c Context.cpp $(clangFlags)
TempBuilder.o:
	clang++ -c TempBuilder.cpp $(clangFlags)
ConExpr.o:
	clang++ -c ConExpr.cpp $(clangFlags)
WhileBuilder.o:
	clang++ -c WhileBuilder.cpp $(clangFlags)
CooperativeCache.o:
	clang++ -c CooperativeCache.cpp $(clangFlags)

run: test
	PGHOST='127.0.0.1' PGPORT='5432' PGDATABASE='DataStore' PGUSER='research' PGPASSWORD='swimsand' ./test

debug: test
	PGHOST='127.0.0.1' PGPORT='5432' PGDATABASE='DataStore' PGUSER='research' PGPASSWORD='swimsand'  gdb ./test

testing: utils.o Tracker.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o
	export TRANS_DIR=$(TRANS_DIR); $(MAKE) -C tests

sql_testing: utils.o Tracker.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o
	export TRANS_DIR=$(TRANS_DIR); cd tests && $(MAKE) sql


clean:
	rm *.o; rm chat; rm test; rm stress1; rm stress2; rm stress3
