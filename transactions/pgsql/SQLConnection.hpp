#pragma once
#include "test_utils.hpp"
#include "Basics.hpp"
#include "batched_connection.hpp"
#include <pqxx/pqxx>
#include <mutex>
#include <resource_pool.hpp>
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
#ifndef NDEBUG
			void onAcquire(...){
				assert(!current_trans);
			}

			void onRelease(){
				assert(!current_trans);
			}
#endif
			connection conn;
			SQLTransaction* current_trans{nullptr};
			bool in_trans() const;
			SQLConnection(connection conn):conn{std::move(conn)}{}
		};
		
		using LockedSQLConnection = mutils::ResourcePool<SQLConnection>::LockedResource;
		using WeakSQLConnection = mutils::ResourcePool<SQLConnection>::WeakResource;

		//should be a singleton per level
		template<Level l>
		struct SQLConnectionPool{

#ifndef NDEBUG
			static bool constructed;
			SQLConnectionPool(){
				assert(!constructed);
				constructed = true;
			}
#endif
			
			connections bc{
				(l == Level::strong ? strong_ip_addr : causal_ip_addr),
					(l == Level::strong ? strong_sql_port : causal_sql_port),(MAX_THREADS/2)};
			mutils::ResourcePool<SQLConnection> rp{MAX_THREADS,MAX_THREADS,
					[this]{return new SQLConnection(bc.spawn());}};
			LockedSQLConnection acquire(){
				return rp.acquire();
			}
		};
	} }
