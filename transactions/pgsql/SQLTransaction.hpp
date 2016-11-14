#pragma once
#include "SQLConnection.hpp"
#include <iostream>
#include <fstream>

namespace myria{

	struct SerializationFailure : mutils::MyriaException {
		const std::string why;
		SerializationFailure(const std::string why)
			:why(why){}
		virtual const char* what() const noexcept {
			return why.c_str();
		}
	};
	
	namespace pgsql {


		struct SQLTransaction {
			
			GDataStore& gstore;
			const std::string level_string = (gstore.level == Level::strong ? "strong" : "causal");
		private:
			LockedSQLConnection sql_conn;
			bool remote_aborted{false};
			
#ifndef NDEBUG
			std::ofstream &log_file{sql_conn->conn->get_log_file()};
			
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
				sql_conn->conn->send(trans,name,args...);
			}

			void handle_serialization_failure(mutils::ControlChannel& _cc){
				mutils::connection& cc = _cc;
				mutils::connection& dc = _cc.data_channel;
				long int failure_nonce{-1};
				cc.receive(failure_nonce);
				cc.send(failure_nonce);
				char failure_bytes[sizeof(failure_nonce)];
				char const * const falure_bytes_reference = (char*)&failure_nonce;
				//ensure the failure_bytes do not accidentally match the reference
				for (std::size_t i = 0; i < sizeof(failure_nonce); ++i){
					failure_bytes[i] = falure_bytes_reference[i]+1;
				}
				//drain the data socket until we find the failure nonce.
				for (std::size_t i = 0; i < sizeof(failure_nonce);){
					dc.receive(failure_bytes[i]);
					if (failure_bytes[i] == falure_bytes_reference[i]) ++i;
				}
				mutils::DeserializationManager *dsm{nullptr};
				std::size_t why_size{0};
				cc.receive(why_size);
				remote_aborted = true;
				throw SerializationFailure(*cc.template receive<std::string>(dsm,why_size));
			}

			template<typename... T>
			void receive(const std::string & whendebug(what), T& ... t){
				whendebug(log_receive_start(what);)
					try {
						sql_conn->conn->receive(t...);
					}
					catch (mutils::ControlChannel& cc){
						handle_serialization_failure(cc);
					}
				whendebug(log_receive_stop(what);)
			}
			
			template<typename... T>
			void receive_data(const std::string & whendebug(what), const T& ... t){
				whendebug(log_receive_start(what);)
					try {
						sql_conn->conn->receive_data(t...);
					}
					catch (mutils::ControlChannel& cc){
						handle_serialization_failure(cc);
					}
				whendebug(log_receive_stop(what);)
			}


			bool exists(Name n){
				prepared(level_string + " exists",TransactionNames::exists,n);
				bool b{false};
				receive("exists", b);
				return b;
			}

			void Del(Name n){
				prepared("del",TransactionNames::Del,n);
			}
			
			template<typename Vers>
			void select_version(Table t, Name n, Vers& vers){
				prepared(level_string + " select version", TransactionNames::select_version, t,n);
				receive(level_string + " select version",vers);
			}

			template<typename Vers>
			std::vector<char> select_version_data(Table t, Name n, Vers& vers){
				prepared(level_string + " selct_version_data",TransactionNames::select_version_data,t,n);
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
				receive_data(level_string + " receive data", v.size(),v.data());
				return v;
			}
			template<typename Data, typename Vers>
			void update_data(Table t, Name n, Data &d, Vers& vers){
				assert(gstore.level == Level::strong);
				prepared("update_data, strong",TransactionNames::update_data, t,n,d);
				receive("update_data version, strong", vers);
			}

			template<typename RG, typename Data, typename Vers, typename Clock>
			void update_data(Table t, const RG& rg, Name n, const Clock& c, Data &d, Vers& vers){
				assert(gstore.level == Level::causal);
				prepared("update_data, causal",TransactionNames::update_data,t,rg,n,c,d);
				receive("update_data version, causal", vers);
			}

			template<typename Blob>
			void initialize_with_id(Table t, Name id, const Blob &b) {
				assert(gstore.level == Level::strong);
				prepared("initialize_with_id, strong",TransactionNames::initialize_with_id,t,id,b);
			}
			
			template<typename RG, typename Clock, typename Blob>
			void initialize_with_id(Table t, const RG& rg, Name id, const Clock& c, const Blob &b){
				assert(gstore.level == Level::causal);
				prepared("initialize_with_id, causal",TransactionNames::initialize_with_id,t,rg,id,c,b);
			}

			template<typename Vers>
			void increment(Name n, Vers& vers){
				assert(gstore.level == Level::strong);
				prepared("increment, strong",TransactionNames::increment,n);
				receive("increment, strong", vers);
			}

			template<typename RG, typename Vers, typename Clock>
			void increment(const RG& rg, Name n, const Clock& c, Vers& vers){
				assert(gstore.level == Level::causal);
				prepared("increment, causal",TransactionNames::increment,rg,n,c);
				receive("increment, causal", vers);
			}


			bool store_commit(){
				char trans{0};
				log_send(level_string + " commit");
				sql_conn->conn->send(trans);
				//we actually do need to block until commits happen
				receive("check committed",trans);
				return true;
			}

			void store_abort();

			std::list<SQLStore_impl::GSQLObject*> objs;
			void add_obj(SQLStore_impl::GSQLObject* gso);

			~SQLTransaction();
		};

	}
}
