#pragma once
#include "test_utils.hpp"
#include <pqxx/pqxx>
#include <mutex>
#include <resource_pool.hpp>

namespace myria{ namespace pgsql {

		struct SQLTransaction;

		enum class TransactionNames{
			exists, Del, select_version, select_version_data,
				update_data,initialize_with_id,increment,
				MAX
		};

		struct WeakSQLConnection{
			mutils::weak_connection conn;
		}

		struct LockedSQLConnection {

			mutils::locked_connection conn;
			LockedSQLConnection(mutils::locked_connection conn)
				:conn(conn){}
			
			SQLTransaction* current_trans = nullptr;
			std::mutex con_guard;
			static const constexpr unsigned int ip_addr{mutils::get_strong_ip()};
			static const constexpr int repl_group{CAUSAL_GROUP};
			bool in_trans(){
				return current_trans;
			}

			LockedSQLConnection(const LockedSQLConnection&) = delete;
			~LockedSQLConnection(){
				assert(~current_trans);
			}
		};
		
		struct SQLConnectionPool {
			mutils::batched_connections bc;
			LockedSQLConnection acquire(){
				return LockedSQLConnection{bc.spawn()};
			}
		};

	} }
