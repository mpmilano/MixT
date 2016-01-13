#pragma once
#include "SQLConnection.hpp"
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
			SQLStore_impl::SQLConnection& sql_conn;
			pqxx::work trans;
		public:
			bool commit_on_delete = false;
			SQLTransaction(GDataStore& store, SQLStore_impl::SQLConnection& c);

			bool is_serialize_error(const pqxx::pqxx_exception &r);
	
			SQLTransaction(const SQLTransaction&) = delete;
	
			template<typename Arg1, typename... Args>
			auto prepared(const std::string &name, const std::string &stmt,
						  Arg1 && a1, Args && ... args);

			pqxx::result exec(const std::string &str);
	
			bool store_commit(); 

			std::list<SQLStore_impl::GSQLObject*> objs;
			void add_obj(SQLStore_impl::GSQLObject* gso);

			~SQLTransaction();
		};

	}}
