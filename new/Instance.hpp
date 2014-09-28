#pragma once
#include "Backend.hpp"
#include "Util.hpp"


typedef int location;
//just declaring things here, so that we can have location type-tags inherited everywhere.

template<location l>
class Instance{

	enum class LSWhen {deferred, immediate};
	template<LSWhen when>
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
	struct is_handle : public is_T<GenericHandle,T> {};

};
