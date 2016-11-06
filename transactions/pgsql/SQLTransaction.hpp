#pragma once
#include "SQLConnection.hpp"
#include <pqxx/pqxx>
#include "BlobUtils.hpp"
#include <iostream>
#include <fstream>

namespace myria{

	struct SerializationFailure : mutils::StaticMyriaException<MACRO_GET_STR("Error: Serialization Failure")> {};
	
	namespace pgsql {


		struct SQLTransaction {
			
			GDataStore& gstore;
			const std::string level_string = (gstore.level == Level::strong ? "strong" : "causal");
		private:
			LockedSQLConnection sql_conn;
			bool remote_aborted{false};
			
#ifndef NDEBUG
			std::ofstream &log_file{sql_conn->conn.log_file};
			
			void log_receive_start(const std::string&);
			void log_receive_stop(const std::string&);
			void log_send(const std::string&);
#else			
#define log_receive_start(...) ;
#define log_receive_stop(...) ;
#define log_send(...) ;
#endif
		public:
#ifndef NDEBUG
			const std::string why;
#endif
			bool commit_on_delete = false;
			SQLTransaction(GDataStore& store, LockedSQLConnection c whendebug(, std::string why));
	
			SQLTransaction(const SQLTransaction&) = delete;
			
			template<typename... Args>
			void prepared(const std::string& whendebug(what), TransactionNames name, Args && ... args){
				char trans{3};
				whendebug(log_send(what);)
				sql_conn->conn.send(trans,name,args...);
			}


			template<typename... T>
			void receive(const std::string & whendebug(what), T& ... t){
				whendebug(log_receive_start(what);)
				sql_conn->conn.receive(t...);
				whendebug(log_receive_stop(what);)
			}

			void check_serialization_failure()  {
				char failed{-1};
				receive(level_string + " check_failure", failed);
				assert(failed != -1);
				assert(failed == 1 || failed == 0);
				if (failed == 1){
					remote_aborted = true;
					throw SerializationFailure();
				}
			}
			
			bool exists(Name n){
				prepared(level_string + " exists",TransactionNames::exists,n);
				check_serialization_failure();
				bool b{false};
				receive("exists", b);
				return b;
			}

			void Del(Name n){
				prepared("del",TransactionNames::Del,n);
				check_serialization_failure();
			}
			
			template<typename Vers>
			void select_version(Table t, Name n, Vers& vers){
				prepared(level_string + " select version", TransactionNames::select_version, t,n);
				check_serialization_failure();
				receive(level_string + " select version",vers);
			}

			template<typename Vers>
			std::vector<char> select_version_data(Table t, Name n, Vers& vers){
				prepared(level_string + " selct_version_data",TransactionNames::select_version_data,t,n);
				check_serialization_failure();
				receive(level_string + " select_version_data",vers);
				std::vector<char> v;
				if (t == Table::BlobStore) {
					std::size_t size{0};
					receive(level_string + " blob_store size",size);
					v.resize(size);
				}
				else {
					v.resize(sizeof(int));
					
				}
				log_receive_start(level_string + " receive data");
				sql_conn->conn.receive(v.size(),v.data());
				log_receive_stop(level_string + " receive data");
				return v;
			}

			template<typename Data, typename Vers>
			void update_data(Table t, Name n, Data &d, Vers& vers){
				assert(gstore.level == Level::strong);
				prepared("update_data, strong",TransactionNames::update_data, t,n,d);
				check_serialization_failure();
				receive("update_data version, strong", vers);
			}

			template<typename RG, typename Data, typename Vers, typename Clock>
			void update_data(Table t, const RG& rg, Name n, const Clock& c, Data &d, Vers& vers){
				assert(gstore.level == Level::causal);
				prepared("update_data, causal",TransactionNames::update_data,t,rg,n,c,d);
				check_serialization_failure();
				receive("update_data version, causal", vers);
			}

			template<typename Blob>
			void initialize_with_id(Table t, Name id, const Blob &b) {
				assert(gstore.level == Level::strong);
				prepared("initialize_with_id, strong",TransactionNames::initialize_with_id,t,id,b);
				check_serialization_failure();
			}
			
			template<typename RG, typename Clock, typename Blob>
			void initialize_with_id(Table t, const RG& rg, Name id, const Clock& c, const Blob &b){
				assert(gstore.level == Level::causal);
				prepared("initialize_with_id, causal",TransactionNames::initialize_with_id,t,rg,id,c,b);
				check_serialization_failure();
			}

			template<typename Vers>
			void increment(Name n, Vers& vers){
				assert(gstore.level == Level::strong);
				prepared("increment, strong",TransactionNames::increment,n);
				check_serialization_failure();
				receive("increment, strong", vers);
			}

			template<typename RG, typename Vers, typename Clock>
			void increment(const RG& rg, Name n, const Clock& c, Vers& vers){
				assert(gstore.level == Level::causal);
				prepared("increment, causal",TransactionNames::increment,rg,n,c);
				check_serialization_failure();
				receive("increment, causal", vers);
			}


			bool store_commit(){
				char trans{0};
				log_send(level_string + " commit");
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
