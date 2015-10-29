#pragma once

#include "Operation.hpp"
#include "DataStore.hpp"
#include "Transaction.hpp"
#include <memory>
#include <vector>

/**
   Information: We are assuming an SQL store which has already been configured
   under the following assumptions:

   (1) There exists a table, BlobStore, with columns (id, data), in which we
   can just throw a binary blob.  This is used for objects which the store
   does not know how to handle.

 */

template<Level l>
class SQLStore;

struct SQLStore_impl {
private:
	
	SQLStore_impl(Level);
public:
	virtual ~SQLStore_impl();

	template<Level l>
	friend class SQLStore;

	struct SQLConnection;
	using SQLConnection_t = SQLConnection*;	

	const Level level;
	SQLConnection_t default_connection;
	
	SQLStore_impl(const SQLStore_impl&) = delete;
	
	std::unique_ptr<TransactionContext> begin_transaction();
	
	int instance_id() const {assert(level == Level::strong); return 0; }
	bool exists(int id);
	void remove(int id);

	int ds_id() const;
	static constexpr int ds_id_nl(){ return 2;}
	
	struct GSQLObject {
	private:
		struct Internals;
		Internals* i;
		GSQLObject(int id, int size);
	public:
		GSQLObject(SQLStore_impl &ss, const std::vector<char> &c);
		GSQLObject(SQLStore_impl &ss, int name, const std::vector<char> &c);
		GSQLObject(SQLStore_impl &ss, int name, int size);
		GSQLObject(SQLStore_impl &ss, int name);
		GSQLObject(const GSQLObject&) = delete;
		GSQLObject(GSQLObject&&);
		void save();
		char* load();
		char* obj_buffer();
		SQLStore_impl& store();

		//required by GeneralRemoteObject
		void setTransactionContext(TransactionContext*);
		TransactionContext* currentTransactionContext();
		bool ro_isValid() const;
		int store_instance_id() const;
		int name() const;;

		//required by ByteRepresentable
		int bytes_size() const;
		int to_bytes(char*) const;
		static GSQLObject from_bytes(char* v);
		virtual ~GSQLObject();
	};

};
