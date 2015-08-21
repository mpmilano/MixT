all:
	clang++ list_example.cpp -stdlib=libc++ --std=c++14 -lboost_serialization -lboost_serialization-mt -Wall -o test -ferror-limit=2 -DSTORE_LIST='FileStore<Level::causal>,FileStore<Level::strong>' -DDECLARED_OPERATIONS="DECLARE_OPERATION(Increment, RemoteObject<int>*) template<typename T> DECLARE_OPERATION(Insert, RemoteObject<std::set<T> >*, const T& ) "
