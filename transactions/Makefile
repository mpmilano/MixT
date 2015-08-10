all:
	clang++ new_test.cpp -stdlib=libc++ --std=c++1z -lboost_serialization -lboost_serialization-mt -Wall -o test -ferror-limit=2 -DSTORE_LIST='FileStore<Level::causal>,FileStore<Level::strong>' -DDECLARED_OPERATIONS="template<typename T> DECLARE_OPERATION(Insert, RemoteObject<std::set<T> >*, const T& )"
