#pragma once
#include "SQLConnection.hpp"
#include "myria_utils.hpp"
#include <pqxx/pqxx>

namespace myria{ namespace pgsql {

		

		template<typename E>
		auto exec_prepared_hlpr(E &e){
			return e.exec();
		}

		template<typename E, typename A, typename... B>
		auto exec_prepared_hlpr(E &e, A&& a, B && ... b){
			auto fwd = e(std::forward<A>(a));
			return exec_prepared_hlpr(fwd,std::forward<B>(b)...);
		}


		struct SQLTransaction {
			
			GDataStore& gstore;
		private:
			whennopool(SQLStore_impl &parent;)
			LockedSQLConnection sql_conn;
			std::unique_lock<std::mutex> conn_lock;
			pqxx::work trans;
		public:
			const std::string why;
			bool commit_on_delete = false;
			SQLTransaction(SQLStore_impl &parent, GDataStore& store, LockedSQLConnection c, std::string why);

			bool is_serialize_error(const pqxx::pqxx_exception &r);
	
			SQLTransaction(const SQLTransaction&) = delete;
	
			template<typename Arg1, typename... Args>
			auto prepared(TransactionNames name, const std::string &stmt,
						  Arg1 && a1, Args && ... args);

			pqxx::result exec(const std::string &str);
	
			bool store_commit();

			void store_abort();

			std::list<SQLStore_impl::GSQLObject*> objs;
			void add_obj(SQLStore_impl::GSQLObject* gso);

			~SQLTransaction();
		};

		#define default_sqltransaction_catch									\
		catch(const pqxx::pqxx_exception &r){							\
			commit_on_delete = false;									\
			if (is_serialize_error(r)) throw SerializationFailure{"Serialization Failure"}; \
			else std::rethrow_exception(std::current_exception());						\
		}

		template<typename Arg1, typename... Args>
		auto SQLTransaction::prepared(TransactionNames name, const std::string &stmt,
									  Arg1 && a1, Args && ... args){
			auto nameint = (int) name;
			auto namestr = std::to_string(nameint);
			try{
				if (!sql_conn->prepared.at(nameint)){
					sql_conn->conn.prepare(namestr,stmt);
					sql_conn->prepared[nameint] = true;
				}
				auto fwd = trans.prepared(namestr)(std::forward<Arg1>(a1));
				
				return exec_prepared_hlpr(fwd,std::forward<Args>(args)...);
			}
			default_sqltransaction_catch
		}

	}}
