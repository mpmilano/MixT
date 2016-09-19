#pragma once
#include "SQLConnection.hpp"
#include <pqxx/pqxx>

namespace myria{ namespace pgsql {


		struct SQLTransaction {
			
			GDataStore& gstore;
		private:
			LockedSQLConnection sql_conn;
			std::unique_lock<std::mutex> conn_lock;
		public:
			const std::string why;
			bool commit_on_delete = false;
			SQLTransaction(GDataStore& store, LockedSQLConnection c, std::string why);

			bool is_serialize_error(const pqxx::pqxx_exception &r) const;
	
			SQLTransaction(const SQLTransaction&) = delete;
			
			template<typename... Args>
			prepared(TransactionNames name, Args && ... args){
				char trans{3};
				sql_conn.conn->send(trans,name,args...);
			}

			template<typename... T>
			void receive(T& ... t){
				sql_conn.conn->receive(t...);
			}
			
			bool exists(Name n){
				prepared(TransactionNames::exists,n);
				bool b{false};
				receive(b);
				return b;
			}

			void Del(Name n){
				prepared(TransactionNames::Del,n);
			}
			
			template<typename Vers>
			void select_version(Table t, Name n, Vers& vers){
				prepared(TransactionNames::select_version, t,n);
				receive(vers);
			}

			template<typename Vers>
			std::vector<char> select_version_data(Table t, Name n, Vers& vers){
				prepared(TransactionNames::select_version_data,t,n);
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
				sql_conn.conn->receive(v.size(),v.data());
				return v;
			}

			template<typename Data, typename Vers>
			void update_data(Table t, Name n, Data &d, Vers& vers){
				prepared(TransactionNames::update_data, t,n,d);
				receive(vers);
			}

			template<typename RG, typename Data, typename Vers, typename Clock>
			void update_data(Table t, const RG& rg, Name n, const Clock& c, Data &d, Vers& vers){
				prepared(TransactionNames::update_data,t,rg,n,c,d);
				receive(vers);
			}

			template<typename Blob>
			void initialize_with_id(Table t, Name id, const Blob &b){
				prepared(TransactionNames::initialize_with_id,t,id,b);
			}
			
			template<typename RG, typename Clock, typename Blob>
			void initialize_with_id(Table t, const RG& rg, Name id, const Clock& c, const Blob &b){
				prepared(TransactionNames::initialize_with_id,t,rg,id,c,b);
			}

			template<typename Data, typename Vers>
			void increment(Table t, Name n, Data &d, Vers& vers){
				prepared(TransactionNames::increment, t,n,d);
				receive(vers);
			}

			template<typename RG, typename Data, typename Vers, typename Clock>
			void increment(Table t, const RG& rg, Name n, const Clock& c, Data &d, Vers& vers){
				prepared(TransactionNames::increment,t,rg,n,c,d);
				receive(vers);
			}

			auto exec(const std::string &str){
				char trans{2};
				sql_conn.conn->send(trans,str.c_str());
				return sql_conn.conn->receive_tpl<Tpl>();
			}

			void exec_noresponse(const std::string &str){
				char trans{2};
				sql_conn.conn->send(trans,str.c_str());
			}
	
			bool store_commit(){
				char trans{0};
				sql_conn.conn->send(trans);
				return true;
			}

			void store_abort();

			std::list<SQLStore_impl::GSQLObject*> objs;
			void add_obj(SQLStore_impl::GSQLObject* gso);

			~SQLTransaction();
		};

	}
}
