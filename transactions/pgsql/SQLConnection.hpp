#pragma once
#include "test_utils.hpp"
#include "Basics.hpp"
#include <pqxx/pqxx>
#include <mutex>
#include <resource_pool.hpp>
#include "proxy_connection.hpp"
#include "SQLConstants.hpp"

namespace myria{ namespace pgsql {

		struct SQLTransaction;

		enum class TransactionNames{
			exists, Del, select_version, select_version_data,
				update_data,initialize_with_id,increment,
				MAX
		};

		struct LockedSQLConnection;

		using SQLTransaction_p = SQLTransaction*;

		using mutils::proxy_connection::weak_connection;
		using mutils::proxy_connection::locked_connection;
		using mutils::proxy_connection::connections;

		struct WeakSQLConnection{
			weak_connection conn;
			
			LockedSQLConnection acquire_if_locked() const;
			LockedSQLConnection lock();
			WeakSQLConnection(weak_connection conn);
			WeakSQLConnection(LockedSQLConnection &l);
			WeakSQLConnection(LockedSQLConnection &&l);
		};

		using void_p = void*;
		
		struct LockedSQLConnection {

			locked_connection conn;
			LockedSQLConnection(locked_connection conn);

			SQLTransaction_p current_trans() const;

			void_p& current_trans_vp();
			
			bool in_trans();

			LockedSQLConnection(const LockedSQLConnection&) = delete;
			LockedSQLConnection(LockedSQLConnection&& o);
		};



		template<Level l>
		struct SQLConnectionPool {
			connections bc{
				(l == Level::strong ? ip_addr : 0),
					(l == Level::strong ? strong_sql_port : causal_sql_port),(MAX_THREADS*2)
					};
			LockedSQLConnection acquire(){
				return bc.spawn();
			}
			template<typename duration>
			auto find_abandoned(const duration& d){
				return bc.abandoned_conections(d);
			}
		};

	} }
