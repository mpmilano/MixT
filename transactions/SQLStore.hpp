#pragma once

#include "Operation.hpp"
#include "DataStore.hpp"
#include "RemoteObject.hpp"

/**
   Information: We are assuming an SQL store which has already been configured
   under the following assumptions:

   (1) There exists a table, BlobStore, with columns (id, data), in which we
   can just throw a binary blob.  This is used for objects which the store
   does not know how to handle.

 */

struct SQLStore : public DataStore<Level::strong> {
private:
	SQLStore(){}
public:
	SQLStore(const SQLStore&) = delete;
	static SQLStore& inst(){
		static SQLStore ss;
		return ss;
	}

	using id = std::integral_constant<int,2>;


	template<typename T>
	struct SQLObject : public RemoteObject<T> {
		
	};
};
