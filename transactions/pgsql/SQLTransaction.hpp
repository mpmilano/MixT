#pragma once
#include "SQLConnection.hpp"
#include <pqxx/pqxx>
#include "BlobUtils.hpp"

namespace myria{

	struct SerializationFailure : mutils::StaticMyriaException<MACRO_GET_STR("Error: Serialization Failure")> {};
	
	namespace pgsql {


		struct SQLTransaction {
			
			GDataStore& gstore;
		private:
			LockedSQLConnection sql_conn;
			bool remote_aborted{false};
		public:
			const std::string why;
			bool commit_on_delete = false;
			SQLTransaction(GDataStore& store, LockedSQLConnection c, std::string why);
	
			SQLTransaction(const SQLTransaction&) = delete;
			
			template<typename... Args>
			void prepared(TransactionNames name, Args && ... args){
				char trans{3};
				sql_conn->conn.send(trans,name,args...);
			}


			template<typename... T>
			void receive(T& ... t){
				sql_conn->conn.receive(t...);
			}

			void check_serialization_failure()  {
				char failed{-1};
				receive(failed);
				assert(failed != -1);
				assert(failed == 1 || failed == 0);
				if (failed == 1){
					remote_aborted = true;
					throw SerializationFailure();
				}
			}
			
			bool exists(Name n){
				prepared(TransactionNames::exists,n);
				check_serialization_failure();
				bool b{false};
				receive(b);
				return b;
			}

			void Del(Name n){
				prepared(TransactionNames::Del,n);
				check_serialization_failure();
			}
			
			template<typename Vers>
			void select_version(Table t, Name n, Vers& vers){
				prepared(TransactionNames::select_version, t,n);
				check_serialization_failure();
				receive(vers);
			}

			template<typename Vers>
			std::vector<char> select_version_data(Table t, Name n, Vers& vers){
				prepared(TransactionNames::select_version_data,t,n);
				check_serialization_failure();
				receive(vers);
				std::vector<char> v;
				if (t == Table::BlobStore) {
					std::size_t size{0};
					receive(size);
					v.resize(size);
				}
				else {
					v.resize(sizeof(int));
					
				}
				sql_conn->conn.receive(v.size(),v.data());
				return v;
			}

			template<typename Data, typename Vers>
			void update_data(Table t, Name n, Data &d, Vers& vers){
				assert(gstore.level == Level::strong);
				prepared(TransactionNames::update_data, t,n,d);
				check_serialization_failure();
				receive(vers);
			}

			template<typename RG, typename Data, typename Vers, typename Clock>
			void update_data(Table t, const RG& rg, Name n, const Clock& c, Data &d, Vers& vers){
				assert(gstore.level == Level::causal);
				prepared(TransactionNames::update_data,t,rg,n,c,d);
				check_serialization_failure();
				receive(vers);
			}

			template<typename Blob>
			void initialize_with_id(Table t, Name id, const Blob &b) {
				assert(gstore.level == Level::strong);
				prepared(TransactionNames::initialize_with_id,t,id,b);
				check_serialization_failure();
			}
			
			template<typename RG, typename Clock, typename Blob>
			void initialize_with_id(Table t, const RG& rg, Name id, const Clock& c, const Blob &b){
				assert(gstore.level == Level::causal);
				prepared(TransactionNames::initialize_with_id,t,rg,id,c,b);
				check_serialization_failure();
			}

			template<typename Vers>
			void increment(Name n, Vers& vers){
				assert(gstore.level == Level::strong);
				prepared(TransactionNames::increment,n);
				check_serialization_failure();
				receive(vers);
			}

			template<typename RG, typename Vers, typename Clock>
			void increment(const RG& rg, Name n, const Clock& c, Vers& vers){
				assert(gstore.level == Level::causal);
				prepared(TransactionNames::increment,rg,n,c);
				check_serialization_failure();
				receive(vers);
			}


			bool store_commit(){
				char trans{0};
				sql_conn->conn.send(trans);
				check_serialization_failure();
				return true;
			}

			void store_abort();

			std::list<SQLStore_impl::GSQLObject*> objs;
			void add_obj(SQLStore_impl::GSQLObject* gso);

			~SQLTransaction();
		};

	}
}
