all:
	clang++ new_test.cpp -stdlib=libc++ --std=c++14 -I/home/research/boost/boost_1_56_0 -L/home/research/boost/boost_1_56_0/stage/lib -lboost_serialization -lboost_serialization-mt -lboost_system -lboost_filesystem -Wall -o test -ferror-limit=1 -DSTORE_LIST='FileStore<Level::causal>,FileStore<Level::strong>' -DDECLARED_OPERATIONS="DECLARE_OPERATION(Increment, RemoteObject<int>*) template<typename T> DECLARE_OPERATION(Insert, RemoteObject<std::set<T> >*, const T& ) "

run:
	LD_LIBRARY_PATH=/home/research/boost/boost_1_56_0/stage/lib ./test
