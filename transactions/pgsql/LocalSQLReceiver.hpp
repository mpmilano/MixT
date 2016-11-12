#pragma once
#include "LocalSQLConnection.hpp"
#include "LocalSQLTransaction.hpp"
#include "SQLConstants.hpp"
#include "dual_connection.hpp"
#include "pgexceptions.hpp"

namespace myria {
	namespace pgsql {
		namespace local {

			using conn_space::receiver;
			
			template<Level l>
			class SQLReceiver{
			public:
				
				mutils::dual_connection_receiver<receiver> r;

				using sizes_t = std::vector<std::size_t>;

				static mutils::dualstate_action_t new_connection(whendebug(std::ofstream& log_file,) ::mutils::connection& data, ::mutils::connection& control) {
					struct ReceiverFun : public mutils::dual_state_receiver {
						whendebug(std::ofstream& log_file;)
						std::unique_ptr<LocalSQLConnection<l> > db_connection{
							new LocalSQLConnection<l>()};
						std::unique_ptr<LocalSQLTransaction<l> > current_trans{nullptr};

						mutils::connection& data_conn;
						mutils::connection& control_conn;

						int underlying_fd() {
							return (db_connection ?
									db_connection->underlying_fd() :
									current_trans->conn->underlying_fd());
						}

						void async_tick() {
							try {
								if (db_connection){
									db_connection->tick();
								}
								else {
									current_trans->conn->tick();
								}
							}
							catch (const SerializationFailure& sf){
								char serialization_failure{1};
								control_conn.send(serialization_failure);
								db_connection = current_trans->store_abort(std::move(current_trans),control_conn);
							}
							catch (const SQLFailure& sf){
								std::cerr << sf.what() << std::endl;
								assert(false && "fatal SQL error");
								struct diedie{}; throw diedie{};
							}
						}
						
					void deliver_new_data_event(const void* data){
							const char* _data = (const char*) data;
							if (!current_trans) {
								
								if (_data[0] != 4 && _data[0] != 1){
									std::cout << (int) _data[0] << std::endl;
								}
								assert(_data[0] == 4 || _data[0] == 1);
								
								
								if (_data[0] == 4){
									assert(!current_trans);
									assert(db_connection);
									current_trans.reset(new LocalSQLTransaction<l>(std::move(db_connection) whendebug(, log_file)));
								}
								else {
									//if we're aborting a non-existant transaction, there's nothing to do.
#ifndef NDEBUG
									log_file << "aborting non-existant transaction" << std::endl;
									log_file.flush();
#endif
								}
							}
							else if (_data[0] == 0){
								//we're finishing this transaction
								db_connection = current_trans->store_commit(std::move(current_trans),data_conn);
								assert(db_connection);
							}
							else if (_data[0] == 1){
								//we're aborting this transaction
								db_connection = current_trans->store_abort(std::move(current_trans),data_conn);
								assert(db_connection);
							}
							else {
								assert(_data[0] != 4);
								assert(_data[0] == 3);
								TransactionNames name = *((TransactionNames*) (_data + 1));
								auto pair = current_trans->receiveSQLCommand(
									std::move(current_trans),
									name, _data + 1 + sizeof(name),
									data_conn
									);
								current_trans = std::move(pair.first);
								db_connection = std::move(pair.second);
								assert(current_trans || db_connection);
							}

#ifndef NDEBUG
							if (current_trans){
								current_trans->log_file << "done processing this request" << std::endl;
								current_trans->log_file.flush();
							}
							else {
								log_file << "done processing this request; transaction was destroyed"
										 << std::endl;
							}
#endif
						}

						void deliver_new_control_event(const void*){
							assert(false && "this channel was not supposed to be used...");
						}
						
						ReceiverFun(ReceiverFun&& o)
							:whendebug(log_file(o.log_file),)
							db_connection(std::move(o.db_connection)),
							current_trans(std::move(o.current_trans)),
							data_conn(o.data_conn),
							control_conn(o.control_conn)
							{}
						
						ReceiverFun(whendebug(std::ofstream& log_file,) ::mutils::connection& data, ::mutils::connection& control)
							:whendebug(log_file(log_file),)
							 data_conn(data),control_conn(control)
							{}
					};
					return mutils::dualstate_action_t{new ReceiverFun(whendebug(log_file,) data, control)};
				}
				
				SQLReceiver():r((l == Level::strong?
								 strong_sql_port :
								 causal_sql_port),
				 				new_connection){}
			};
		}
	}
}
