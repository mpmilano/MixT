#pragma once
#include "test_utils.hpp"
#include "Basics.hpp"
#include "batched_connection.hpp"
#include <mutex>
#include <resource_pool.hpp>
#include "SQLConstants.hpp"
#include "dual_connection.hpp"

namespace myria{ namespace pgsql {

		struct SQLTransaction;

		enum class TransactionNames{
			exists, Del, select_version, select_version_data,
				update_data,initialize_with_id,increment,
				MAX
		};

		struct SQLConnection{
#ifndef NDEBUG
			void onAcquire(...){
				assert(!current_trans);
			}

			void onRelease(){
				assert(!current_trans);
			}
#endif
			std::unique_ptr<mutils::connection> conn;
			SQLTransaction* current_trans{nullptr};
			bool in_trans() const;
			SQLConnection(std::unique_ptr<mutils::connection> conn):conn{std::move(conn)}{}
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

			constexpr unsigned long num_connections(){
				using namespace std::chrono;
				using namespace mutils;
				return 3*(NUM_CLIENTS + (INCREASE_BY*(TEST_STOP_TIME/INCREASE_DELAY)))/4;
			}
			
			mutils::dual_connection_manager<mutils::batched_connection::connections> bc{
				(l == Level::strong ? strong_ip_addr : causal_ip_addr),
					(l == Level::strong ? strong_sql_port : causal_sql_port),(num_connections()/2)};
			
			mutils::ResourcePool<SQLConnection> rp{num_connections()/4,num_connections()/4,
					[this]{
					using subcon = std::decay_t<decltype(bc.spawn())>;
					return new SQLConnection(std::unique_ptr<mutils::connection>(new subcon(bc.spawn())));}};
			LockedSQLConnection acquire(){
				return rp.acquire();
			}
		};
	} }
