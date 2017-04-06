#pragma once
#include "test_utils.hpp"
#include <pqxx/pqxx>
#include <mutex>
#include <resource_pool.hpp>

namespace myria{ namespace pgsql {

		struct SQLTransaction;

		enum class TransactionNames{
			exists, Del1, Del2, select_version_s_i, select_version_s_b,
				select1, select2, Updates1, Updates2, Increment, Insert1, Insert2,
				Sel1i,Sel1b,udc1,udc2,udc3,udc4,udc5,udc6,udc7,udc8,
				ic1,ic2,ic3,ic4,ic5,ic6,ic7,ic8,initci,initcb,
				MAX
		};

		struct SQLConnection {
			
			std::vector<bool> prepared;
			
#ifndef NDEBUG
			void onAcquire(...){
				assert(!current_trans);
			}

			void onRelease(){
				assert(!current_trans);
			}
#endif
			
			SQLTransaction* current_trans = nullptr;
			std::mutex con_guard;
			static const constexpr int repl_group{CAUSAL_GROUP};
			bool in_trans() const ;
	
			//hoping specifying nothing means
			//env will be used.
			pqxx::connection conn;
			SQLConnection(std::string host);
			SQLConnection(const SQLConnection&) = delete;
		};

		constexpr unsigned long num_sql_connections(){
			using namespace std::chrono;
				using namespace mutils;
				return 4*(NUM_CLIENTS + (INCREASE_BY*(TEST_STOP_TIME/INCREASE_DELAY)));
			}

#ifndef NOPOOL
#define whenpool(x...) x
#define whennopool(x...)
		
		template<Level l>
		struct SQLConnectionPool : public mutils::ResourcePool<SQLConnection>{

			static auto* newsqlconn(){
				return new SQLConnection(mutils::string_of_ip(
																	 l == Level::strong ? 
																	 mutils::get_strong_ip() :
																	 mutils::get_causal_ip()
																	 ));
			}
			
			SQLConnectionPool()
				:ResourcePool<SQLConnection>(3*num_sql_connections()/4,num_sql_connections()/4,newsqlconn,false)
				{}
		};

		struct LocalSQLConnectionPool : public mutils::ResourcePool<SQLConnection>{

			static auto* newsqlconn(){
				return new SQLConnection("/run/postgresql");
			}
			
			LocalSQLConnectionPool()
				:ResourcePool<SQLConnection>(3*num_sql_connections()/4,num_sql_connections()/4,newsqlconn,false)
				{}
		};
		using GeneralSQLConnectionPool = mutils::ResourcePool<SQLConnection>;
		using WeakSQLConnection = typename GeneralSQLConnectionPool::WeakResource;
		using LockedSQLConnection = typename GeneralSQLConnectionPool::LockedResource;
#else
#define whenpool(x...)
#define whennopool(x...) x
		using WeakSQLConnection = std::unique_ptr<SQLConnection>;
		using LockedSQLConnection = WeakSQLConnection;
#endif

	} }
