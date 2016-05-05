#pragma once
#include "SQLStore_impl.hpp"
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

		struct SQLStore_impl::SQLConnection {
			
			std::vector<bool> prepared;
			
			SQLTransaction* current_trans = nullptr;
			std::mutex con_guard;
			const int ip_addr;
			const int repl_group;
			bool in_trans();
	
			//hoping specifying nothing means
			//env will be used.
			pqxx::connection conn;
			SQLConnection(int ip);
			SQLConnection(const SQLConnection&) = delete;
		};

	} }
