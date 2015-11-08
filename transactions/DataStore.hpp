#pragma once

#include "Operation.hpp"
#include "GDataStore.hpp"
#include "Basics.hpp"

template<Level l>
class DataStore : public GDataStore{
public:

	DataStore():GDataStore{l}{}
	
	//DECLARED_OPERATIONS
	template<Level l2> DECLARE_OPERATION(Increment, RemoteObject<l2,int>*);
	template<Level l2, typename T> DECLARE_OPERATION(Insert, RemoteObject<l2,std::set<T> >*, const T& ) 

};
