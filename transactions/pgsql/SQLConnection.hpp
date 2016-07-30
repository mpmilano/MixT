#pragma once
#include "SQLStore_impl.hpp"
#include "test_utils.hpp"
#include <pqxx/pqxx>
#include <mutex>

namespace myria{ namespace pgsql {

		struct SQLTransaction;

		enum class TransactionNames{
			exists, Del1, Del2, select_version_s_i, select_version_s_b,
				select1, select2, Updates1, Updates2, Increment, Insert1, Insert2,
				Sel1i,Sel1b,udc1,udc2,udc3,udc4,udc5,udc6,udc7,udc8,
				ic1,ic2,ic3,ic4,ic5,ic6,ic7,ic8,initci,initcb,
				MAX
				};

		class WeakSQLReference;
		struct SQLStore_impl::LockedSQLConnection{
			struct Internals;
			Internals *i;
			std::shared_ptr<std::size_t> use_count;
			LockedSQLConnection(std::unique_ptr<SQLConnection>);
			LockedSQLConnection(LockedSQLConnection&& o);
			SQLConnection* operator->();
			SQLConnection& operator*();
			const SQLConnection * operator->() const;
			const SQLConnection& operator*() const;
			operator bool() const {return i;}
			~LockedSQLConnection();
			friend class WeakSQLReference;
		private:
			LockedSQLConnection(const LockedSQLConnection&);
		};

		struct SQLStore_impl::SQLConnection {
			
			std::vector<bool> prepared;
			
			SQLTransaction* current_trans = nullptr;
			SQLStore_impl* current_store = nullptr;
			std::mutex con_guard;
			static const constexpr unsigned int ip_addr{mutils::get_strong_ip()};
			static const constexpr int repl_group{CAUSAL_GROUP};
			bool in_trans();
	
			//hoping specifying nothing means
			//env will be used.
			pqxx::connection conn;
			SQLConnection();
			SQLConnection(const SQLConnection&) = delete;
			~SQLConnection();
		};

	} }
