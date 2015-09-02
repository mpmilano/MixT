#pragma once

#include "Operation.hpp"
#include "DataStore.hpp"

/**
   Information: We are assuming an SQL store which has already been configured
   under the following assumptions:

   (1) There exists a table, BlobStore, with columns (id, data), in which we
   can just throw a binary blob.  This is used for objects which the store
   does not know how to handle.

 */

struct SQLStore : public DataStore<Level::strong> {
private:
	
	SQLStore();

	struct SQLTransaction;

	struct SQLConnection {
		bool in_trans = false;
		SQLConnection() = default;
		SQLConnection(const SQLConnection&) = delete;
	};

	
public:
	SQLConnection default_connection;
	
	SQLStore(const SQLStore&) = delete;
	
	static SQLStore& inst();

	std::unique_ptr<TransactionContext> begin_transaction();
	
	using id = std::integral_constant<int,2>;

	template<typename>
	struct SQLObject;
};

#include "SQLStore_impl.hpp"
