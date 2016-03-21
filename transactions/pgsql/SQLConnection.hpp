#pragma once
#include "SQLStore_impl.hpp"
#include <pqxx/pqxx>
#include <mutex>

namespace myria{ namespace pgsql {

		struct SQLTransaction;


		struct SQLStore_impl::SQLConnection {
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
