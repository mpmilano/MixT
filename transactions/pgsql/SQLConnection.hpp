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

		using mutils::batched_connection::connection;
		using mutils::batched_connection::connections;

		struct SQLConnection{
			void onAcquire(...){
				assert(!current_trans);
			}

			void onRelease(){
				assert(!current_trans);
			}
			connection conn;
			SQLTransaction* current_trans{nullptr};
			bool in_trans() const;
			SQLConnection(connection conn):conn{std::move(conn)}{}
		};
		
		using LockedSQLConnection = mutils::ResourcePool<SQLConnection>::LockedResource;
		using WeakSQLConnection = mutils::ResourcePool<SQLConnection>::WeakResource;

		template<Level l>
		struct SQLConnectionPool{
			connections bc{
				(l == Level::strong ? ip_addr : 0),
					(l == Level::strong ? strong_sql_port : causal_sql_port),(MAX_THREADS/2)
					};
			mutils::ResourcePool<SQLConnection> rp{MAX_THREADS,MAX_THREADS,
					[this]{return new SQLConnection(bc.spawn());}};
			LockedSQLConnection acquire(){
				return rp.acquire();
			}
		};
	} }
