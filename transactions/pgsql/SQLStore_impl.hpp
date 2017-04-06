#pragma once

#include "DataStore.hpp"
#include "cexprutils.hpp"
#include "SQLLevels.hpp"
#include "SQLConnection.hpp"
#include <memory>
#include <vector>
#include <array>

/**
   Information: We are assuming an SQL store which has already been configured
   under the following assumptions:

   (1) There exists a table, BlobStore, with columns (id, data), in which we
   can just throw a binary blob.  This is used for objects which the store
   does not know how to handle.

*/

namespace myria { namespace pgsql {

		template<Level l>
		class SQLStore;

		struct SQLTransaction;

		enum class Table{
			BlobStore = 0,IntStore = 1
				};

		static constexpr int Table_max = 2;
		
		constexpr mutils::CexprString table_name(Table t){
			using namespace mutils;
			constexpr auto bs = "\"BlobStore\"";
			constexpr auto is = "\"IntStore\"";
			switch (t){
			case Table::BlobStore : return CexprString{} + bs;
			case Table::IntStore : return CexprString{} + is;
			};
		}

		struct SQLStore_impl;
		
		struct SQLInstanceManager_abs : public mutils::RemoteDeserializationContext {
			virtual SQLStore<Level::strong>& inst_strong() = 0;
			virtual SQLStore<Level::causal>& inst_causal() = 0;
			SQLStore_impl& inst(Level l);
			
			virtual ~SQLInstanceManager_abs(){}
			SQLInstanceManager_abs(const SQLInstanceManager_abs&) = delete;
			SQLInstanceManager_abs(){}
		};
		
		struct SQLStore_impl {
		private:
	
			SQLStore_impl(whenpool(GeneralSQLConnectionPool& pool) whennopool(const std::string &host), GDataStore &store, /*int instanceID,*/ Level);
			GDataStore &_store;
		public:
			
			virtual ~SQLStore_impl();

			template<Level l>
			friend class SQLStore;

			std::array<int, NUM_CAUSAL_GROUPS> clock;

			const Level level;
			WeakSQLConnection default_connection;
	
			SQLStore_impl(const SQLStore_impl&) = delete;
	
			std::unique_ptr<SQLTransaction> begin_transaction(whendebug(const std::string& why));
			
			bool in_transaction() const;
	
			int instance_id() const;
			bool exists(SQLTransaction*, Name id);
			void remove(SQLTransaction*, Name id);

			int ds_id() const;
			static constexpr int ds_id_nl(){ return 2;}
	
			struct GSQLObject {
				struct Internals;
			private:
				Internals *i;
				GSQLObject(Name id, int size);
			public:
				GSQLObject(SQLStore_impl &ss, Table t, Name name, const std::vector<char> &c);
				GSQLObject(SQLStore_impl &ss, Table t, Name name, int size);
				GSQLObject(SQLStore_impl &ss, Table t, Name name);
				GSQLObject(const GSQLObject&) = delete;
				GSQLObject(GSQLObject&&);
				void save(SQLTransaction*);
				char* load(SQLTransaction*);
				char* obj_buffer();
				char const * obj_buffer() const ;
				void resize_buffer(std::size_t);
				int obj_buffer_size() const;
				const std::array<int,NUM_CAUSAL_GROUPS>& timestamp() const ;
				SQLStore_impl& store();

				//will crash if stored object is non-integral.
				void increment(SQLTransaction*);

				//required by GeneralRemoteObject
				bool ro_isValid(SQLTransaction*) const;
				int store_instance_id() const;
				Name name() const;

				//required by ByteRepresentable
				int bytes_size() const;
				int to_bytes(char*) const;
				static GSQLObject from_bytes(SQLInstanceManager_abs&, char const * v);
				void post_object(const std::function<void (char const * const,std::size_t)>&) const;
				virtual ~GSQLObject();
			};

			//operations

		};

	}}
