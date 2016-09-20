#pragma once
#include "test_utils.hpp"
#include <pqxx/pqxx>
#include <mutex>
#include <resource_pool.hpp>
#include "batched_connection.hpp"

namespace myria{ namespace pgsql {

		struct SQLTransaction;

		enum class TransactionNames{
			exists, Del, select_version, select_version_data,
				update_data,initialize_with_id,increment,
				MAX
		};

		struct WeakSQLConnection{
			mutils::batched_connection::weak_connection conn;
		};

		struct LockedSQLConnection {

			mutils::batched_connection::locked_connection conn;
			LockedSQLConnection(mutils::batched_connection::locked_connection conn)
				:conn(std::move(conn)){}
			
			SQLTransaction* current_trans = nullptr;
			std::unique_ptr<std::mutex> con_guard{new std::mutex()};
			static const constexpr unsigned int ip_addr{mutils::get_strong_ip()};
			static const constexpr int repl_group{CAUSAL_GROUP};
			bool in_trans(){
				return current_trans;
			}

			LockedSQLConnection(const LockedSQLConnection&) = delete;
			LockedSQLConnection(LockedSQLConnection&& o)
				:conn(std::move(o.conn)),
				 current_trans(std::move(o.current_trans)),
				 con_guard(std::move(o.con_guard)){}
			
			~LockedSQLConnection(){
				assert(!current_trans);
			}
		};
		
		struct SQLConnectionPool {
			mutils::batched_connection::batched_connections bc;
			LockedSQLConnection acquire(){
				return bc.spawn();
			}
		};

	} }
