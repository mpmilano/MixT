#pragma once

#include "Operation.hpp"

template<Level>
class DataStore {
public:
	template<typename T>
	DECLARE_OPERATION(Insert, RemoteObject<std::set<T> >*, const T& );

};
