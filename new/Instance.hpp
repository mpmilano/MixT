#pragma once
#include "Backend.hpp"
#include "Util.hpp"
#include <memory>


typedef int location;
//just declaring things here, so that we can have location type-tags inherited everywhere.

template<location l>
class Instance{
public:

	class LogStore;


	class StoredBlob;
	template<typename T> 
	class StoredObject;

	class GenericHandle;
	template<typename T>
	class TypedHandle;
	template<typename T, Level l2, Access::Access a>
	class Handle;

	template<typename T>
	struct is_handle;



};
