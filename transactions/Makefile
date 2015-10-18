TRANS_DIR= `pwd`/
clangFlags= -Xclang -fcolor-diagnostics -g -stdlib=libc++ --std=c++14 -I/home/research/boost/boost_1_56_0 -Wall -ferror-limit=1 -DSTORE_LIST='FileStore<Level::causal>,FileStore<Level::strong>,SQLStore' -DPRINT_TRANS -DHACKY_PREAMBLE="struct SQLStore;" -DDECLARED_OPERATIONS="DECLARE_OPERATION(Increment, RemoteObject<int>*) template<typename T> DECLARE_OPERATION(Insert, RemoteObject<std::set<T> >*, const T& ) "
linkFlags= -stdlib=libc++ --std=c++14 -L/home/research/boost/boost_1_56_0/stage/lib -lpqxx -lboost_serialization -lboost_serialization-mt -lboost_system -lboost_filesystem
export TRANS_DIR
export clangFlags
export linkFlags

all: utils.o dummy_tracker.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o new_test.o
	clang++ utils.o dummy_tracker.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o new_test.o -o test $(linkFlags)

chat: utils.o dummy_tracker.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o chat_example.o
	clang++ utils.o dummy_tracker.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o chat_example.o -o chat $(linkFlags)

new_test.o:
	clang++ -c new_test.cpp $(clangFlags)
chat_example.o:
	clang++ -c chat_example.cpp $(clangFlags)
utils.o:
	clang++ -c utils.cpp $(clangFlags)
dummy_tracker.o:
	clang++ -c dummy_tracker.cpp $(clangFlags)
SerializationSupport.o:
	clang++ -c SerializationSupport.cpp $(clangFlags)
SQLStore.o:
	clang++ -c SQLStore.cpp $(clangFlags)
Context.o:
	clang++ -c Context.cpp $(clangFlags)
TempBuilder.o:
	clang++ -c TempBuilder.cpp $(clangFlags)
ConExpr.o:
	clang++ -c ConExpr.cpp $(clangFlags)
WhileBuilder.o:
	clang++ -c WhileBuilder.cpp $(clangFlags)


run: test
	PGHOST='127.0.0.1' PGPORT='5432' PGDATABASE='DataStore' PGUSER='research' PGPASSWORD='swimsand' LD_LIBRARY_PATH=/home/research/boost/boost_1_56_0/stage/lib ./test

debug: test
	PGHOST='127.0.0.1' PGPORT='5432' PGDATABASE='DataStore' PGUSER='research' PGPASSWORD='swimsand' LD_LIBRARY_PATH=/home/research/boost/boost_1_56_0/stage/lib gdb ./test

testing: utils.o dummy_tracker.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o
	export TRANS_DIR=$(TRANS_DIR); $(MAKE) -C tests

sql_testing: utils.o dummy_tracker.o SerializationSupport.o WhileBuilder.o ConExpr.o Context.o SQLStore.o TempBuilder.o
	export TRANS_DIR=$(TRANS_DIR); cd tests && $(MAKE) sql


clean:
	rm *.o; rm chat; rm test
