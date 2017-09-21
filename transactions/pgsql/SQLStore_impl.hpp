#pragma once

#include "DataStore.hpp"
#include "mutils/cexprutils.hpp"
#include "pgsql/SQLLevels.hpp"
#include "pgsql/SQLConnection.hpp"
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

		template<Level l, typename...>
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
		
		struct SQLStore_impl : public mutils::RemoteDeserializationContext {
		private:
	
			SQLStore_impl(whenpool(GeneralSQLConnectionPool& pool) whennopool(const std::string &host), GDataStore &store, /*int instanceID,*/ Level);
			GDataStore &_store;
		public:
			
			virtual ~SQLStore_impl();

			template<Level l, typename...>
			friend class SQLStore;

			std::array<long long, NUM_CAUSAL_GROUPS> clock;

			const Level level;
			WeakSQLConnection default_connection;
	
			SQLStore_impl(const SQLStore_impl&) = delete;
	
			std::unique_ptr<SQLTransaction> begin_transaction(whendebug(const std::string& why));
			
			bool in_transaction() const;
	
			int instance_id() const;
			bool exists(SQLTransaction*, Name id);
			void remove(SQLTransaction*, Name id);
	
			struct GSQLObject : public mutils::ByteRepresentable{
				struct Internals;
			private:
				Internals *i;
				GSQLObject(Name id, int size);
			public:
				GSQLObject(SQLTransaction* trans, SQLStore_impl &ss, Table t, Name name, const std::vector<char> &c);
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
				const std::array<long long,NUM_CAUSAL_GROUPS>& timestamp() const ;
				SQLStore_impl& store();
				const SQLStore_impl& store() const;

				//will crash if stored object is non-integral.
				void increment(SQLTransaction*);

				//required by GeneralRemoteObject
				bool isValid(SQLTransaction*) const;
				int store_instance_id() const;
				Name name() const;

				//required by ByteRepresentable
				std::size_t bytes_size() const;
				std::size_t to_bytes(char*) const;
				//for constructing from serialized state
				GSQLObject(SQLStore_impl&, char const * v);
				template<typename... ctxs>
				static SQLStore_impl& from_bytes_helper(mutils::DeserializationManager<ctxs...>* m, char const * v){
					int* arr = (int*)v;
					Level* arrl = (Level*) (arr + 3);
					Level lvl = arrl[0];
					SQLStore_impl *sstore{nullptr};
					SQLStore_impl *cstore{nullptr};
					if constexpr(DECT(*m)::template contains_mgr<SQLStore<Level::causal>>()){
							cstore = &m->template mgr<SQLStore<Level::causal>>();
						}
					if constexpr(DECT(*m)::template contains_mgr<SQLStore<Level::strong>>()){
							sstore = &m->template mgr<SQLStore<Level::strong>>();
						}
					return (lvl == Level::causal ? *cstore : *sstore);
				}
				void post_object(const std::function<void (char const * const,std::size_t)>&) const;
				virtual ~GSQLObject();
			};

			//operations

		};

	}}
