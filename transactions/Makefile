all:
	clang++ -g utils.cpp SerializationSupport.cpp SQLStore.cpp list_example.cpp -stdlib=libc++ --std=c++14 -I/home/research/boost/boost_1_56_0 -L/home/research/boost/boost_1_56_0/stage/lib -lpqxx -lboost_serialization -lboost_serialization-mt -lboost_system -lboost_filesystem -Wall -o test -ferror-limit=1 -DSTORE_LIST='FileStore<Level::causal>,FileStore<Level::strong>,SQLStore' -DDECLARED_OPERATIONS="DECLARE_OPERATION(Increment, RemoteObject<int>*) template<typename T> DECLARE_OPERATION(Insert, RemoteObject<std::set<T> >*, const T& ) "

run:
	PGHOST='127.0.0.1' PGPORT='5432' PGDATABASE='DataStore' PGUSER='research' PGPASSWORD='swimsand' LD_LIBRARY_PATH=/home/research/boost/boost_1_56_0/stage/lib ./test

debug:
	PGHOST='127.0.0.1' PGPORT='5432' PGDATABASE='DataStore' PGUSER='research' PGPASSWORD='swimsand' LD_LIBRARY_PATH=/home/research/boost/boost_1_56_0/stage/lib gdb ./test
