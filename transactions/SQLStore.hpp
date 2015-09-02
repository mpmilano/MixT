#pragma once

#include "Operation.hpp"
#include "DataStore.hpp"
#include <memory>

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

	struct SQLConnection;
	using SQLConnection_t = std::unique_ptr<SQLConnection>;
	
public:
	SQLConnection_t default_connection;
	
	SQLStore(const SQLStore&) = delete;
	
	static SQLStore& inst();

	std::unique_ptr<TransactionContext> begin_transaction();
	
	using id = std::integral_constant<int,2>;
	
	struct GSQLObject : public GeneralRemoteObject, public ByteRepresentable {
		struct Internals;
		Internals* i;
		GSQLObject(int size);
		GSQLObject(const GSQLObject&) = delete;
		GSQLObject(GSQLObject&&);
		void save();
		char* load();
		char* update_cached();
		char* obj_buffer();

		//required by GeneralRemoteObject
		void setTransactionContext(TransactionContext*);
		TransactionContext* currentTransactionContext();
		bool isValid() const;
		const GDataStore& store() const;
		GDataStore& store();
		int bytes_size() const;

		//required by ByteRepresentable
		int to_bytes(char*) const;
		static GSQLObject from_bytes(char* v);
		virtual ~GSQLObject();
	};
	

	template<typename T>
	struct SQLObject : public RemoteObject<T> {
		GSQLObject gso;
		std::unique_ptr<T> t;

		SQLObject(GSQLObject gs, std::unique_ptr<T> t):
			gso(std::move(gs)),t(std::move(t)){}
		
		const T& get() {
			//TODO: get really isn't const anywhere.
			//We should remove that designator.

			char * res = nullptr;
			if (t){
				res = gso.update_cached();
			}
			else{
				res = gso.load();
			}
			if (res != nullptr){
				t = ::from_bytes<T>(res);
			}
			return *t;
		}

		void put(const T& t){
			this->t = std::make_unique<T>(t);
			::to_bytes(t,gso.obj_buffer());
			gso.save();
		}

		//these just forward
		void setTransactionContext(TransactionContext* t){
			gso.setTransactionContext(t);
		}
		TransactionContext* currentTransactionContext(){
			return gso.currentTransactionContext();
		}
		bool isValid() const{
			return gso.isValid();
		}
		const GDataStore& store() const{
			return gso.store();
		}
		GDataStore& store(){
			return gso.store();
		}
		int bytes_size() const {
			return gso.bytes_size();
		}
		int to_bytes(char* c) const {
			return gso.to_bytes(c);
		}
	};
	
	template<typename T>
	static std::unique_ptr<SQLObject<T> > from_bytes(char* v){
		return std::make_unique<SQLObject<T> >(GSQLObject::from_bytes(v),
											   std::unique_ptr<T>());
	}


};

#include "SQLStore_impl.hpp"
