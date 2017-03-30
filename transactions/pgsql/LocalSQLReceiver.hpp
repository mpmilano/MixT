#pragma once
#include <sys/sysinfo.h>
#include "LocalSQLConnection.hpp"
#include "LocalSQLTransaction.hpp"
#include "SQLConstants.hpp"
#include "dual_connection.hpp"
#include "pgexceptions.hpp"
#include "Ostreams.hpp"

namespace myria {
	namespace pgsql {
		namespace local {

			using conn_space::receiver;

			struct sql_socket : public mutils::connection {
				mutils::connection& data_conn;
				static constexpr bool success(){ return true;}
				static constexpr bool error() {return false;}
				bool valid() const {return data_conn->valid();}
				std::size_t raw_receive(std::size_t how_many, std::size_t const * const sizes, void ** bufs){
					return data_conn.raw_receive(how_many, sizes,bufs);
				}
				std::size_t raw_send(std::size_t old_how_many, std::size_t const * const old_sizes, void const * const * const old_bufs){
					return send_prepend_extra(data_conn,old_how_many, old_sizes, old_bufs, success());
				}

				std::size_t send_error_string(const std::string &error){
					constexpr std::size_t how_many{3};
					constexpr bool _error = error();
					const std::size_t string_size{error.size()};
					const std::size_t[] sizes = {sizeof(_error), sizeof(string_size), string_size};
					void const *const  bufs[] = {(void*)&_error, (void*) &string_size, (void*) error.c_str()};
					return data_conn.raw_send(how_many, sizes, bufs);
				}
				
#ifndef NDEBUG
				std::ostream& get_log_file() {
					return data_conn.get_log_file();
				}
#endif
				
			};
			
			template<Level l>
			class SQLReceiver{
			public:
				
				mutils::dual_connection_receiver<receiver> r;

				using sizes_t = std::vector<std::size_t>;

				static mutils::dualstate_action_t new_connection(whendebug(std::ostream& log_file,) ::mutils::connection& data) {
					struct ReceiverFun : public mutils::ReceiverFun {
						whendebug(std::ostream& log_file;)
						std::unique_ptr<LocalSQLConnection<l> > db_connection{
							new LocalSQLConnection<l>(whendebug(log_file))};
						std::unique_ptr<LocalSQLTransaction<l> > current_trans{nullptr};

						sql_socket wrapped_conn;
						
						const long int serialization_failure = mutils::long_rand();

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
								wrapped_conn.send_error_string("?");
								db_connection = current_trans->store_abort(std::move(current_trans));
							}
							catch (const SQLFailure& sf){
								std::cerr << sf.what() << std::endl;
								assert(false && "fatal SQL error");
								struct diedie{}; throw diedie{};
							}
						}
						
					void deliver_new_event(const void* data){
						mutils::connection& data_conn = wrapped_conn;
							const char* _data = (const char*) data;
							//request for diagnostics
							if (_data[0] == 6) {
								struct sysinfo reply;
								sysinfo(&reply);
								std::size_t reply_size = mutils::bytes_size(reply);
#ifndef NDEBUG
								std::cout << "memory usage on this host " << std::endl;
								std::cout << l << "l: " << reply.totalram - reply.freeram << std::endl;
#endif
								data_conn.send(reply_size);
								data_conn.send(reply);
							}
							else if (!current_trans) {
								
								if (_data[0] != 4 && _data[0] != 1){
									std::cout << (int) _data[0] << std::endl;
								}
								assert(_data[0] == 4 || _data[0] == 1);
								
								
								if (_data[0] == 4){
									assert(!current_trans);
									assert(db_connection);
									current_trans.reset(new LocalSQLTransaction<l>(std::move(db_connection)));
								}
								else {
									//if we're aborting a non-existant transaction, there's nothing to do.
#ifndef NDEBUG
									auto &log_file = (current_trans ? current_trans->conn->log_file : db_connection->log_file);
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
								db_connection = current_trans->store_abort(std::move(current_trans));
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
								current_trans->conn->log_file << "done processing this request" << std::endl;
								current_trans->conn->log_file.flush();
							}
							else {
								db_connection->log_file << "done processing this request; transaction was destroyed"
										 << std::endl;
							}
#endif
						}
						
						ReceiverFun(ReceiverFun&& o)
							:whendebug(log_file(o.log_file),)
							db_connection(std::move(o.db_connection)),
							current_trans(std::move(o.current_trans)),
							wrapped_conn(o.wrapped_conn)
							{}
						
						ReceiverFun(whendebug(std::ostream& log_file,) ::mutils::connection& data)
							:whendebug(log_file(log_file),)
							 wrapped_conn(data)
							{}
					};
					return mutils::action_t{new ReceiverFun(whendebug(log_file,) data)};
				}
				
				SQLReceiver():r((l == Level::strong?
								 strong_sql_port :
								 causal_sql_port),
				 				new_connection){}
			};
		}
	}
}
