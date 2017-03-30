#pragma once
#include "test_utils.hpp"
#include "Basics.hpp"
#include "batched_connection.hpp"
#include <mutex>
#include <resource_pool.hpp>
#include "SQLConstants.hpp"
#include "dual_connection.hpp"
#include <sys/sysinfo.h>

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
			struct sysinfo collect_machine_stats();
			SQLConnection(std::unique_ptr<mutils::connection> conn):conn{std::move(conn)}{}
		};
		
		using LockedSQLConnection = mutils::ResourcePool<SQLConnection>::LockedResource;
		using WeakSQLConnection = mutils::ResourcePool<SQLConnection>::WeakResource;

		//should be a singleton per level
		template<typename l>
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
				return 4*(NUM_CLIENTS + (INCREASE_BY*(TEST_STOP_TIME/INCREASE_DELAY)));
			}
			
			mutils::batched_connection::connections bc{
				(l::is_strong::value ? strong_ip_addr : causal_ip_addr),
					(l::is_strong::value ? strong_sql_port : causal_sql_port),
					static_cast<int>(num_connections()/2)};

			struct receiver_socket : public mutils::connection{
				using data_conn_t = std::decay_t<decltype(bc.spawn())>;
				data_conn_t data_conn;
				
				receiver_socket(data_conn_t d):data_conn(std::move(d)){}
				
				whendebug(std::ostream& get_log_file(){return data_conn.get_log_file();});
				static constexpr bool success(){ return true;}
				static constexpr bool error() {return false;}
				bool valid() const {return data_conn.valid();}
				std::size_t raw_receive(std::size_t old_how_many, std::size_t const * const old_sizes, void ** old_bufs){
					bool is_success;
					auto ret = receive_prepend_extra(data_conn,old_how_many, old_sizes, old_bufs, is_success);
					//note: do something with the boolean! Also check protocol for symmetry; assumed in this design, bad if not witnessed.
					assert(false);
					return ret;
				}
				std::size_t raw_send(std::size_t how_many, std::size_t const * const sizes, void const * const * const bufs){
					return data_conn.raw_send(how_many, sizes,bufs);
				}
			};
			
			mutils::ResourcePool<SQLConnection> rp{3*num_connections()/4,num_connections()/4,
					[this]{
					using subcon = receiver_socket;
					return new SQLConnection(std::unique_ptr<mutils::connection>(new subcon(bc.spawn())));},false /*disallow overdraws*/};
			LockedSQLConnection acquire(){
				return rp.acquire();
			}
		};
	} }
