all:
	clang++ Blog.cpp -stdlib=libc++ --std=c++1z -I../ -lboost_serialization -lboost_serialization-mt -Wall -o test -ferror-limit=1 -DSTORE_LIST='FileStore<Level::causal>,FileStore<Level::strong>' -DDECLARED_OPERATIONS="template<typename T> DECLARE_OPERATION(Insert, RemoteObject<std::set<T> >*, const T& )"
