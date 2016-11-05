#pragma once
#include "LocalSQLConnection.hpp"
#include "LocalSQLTransaction.hpp"
#include "SQLConstants.hpp"
#include "simple_rpc.hpp"

namespace myria {
	namespace pgsql {
		namespace local {

			namespace conn_space = mutils::batched_connection;
			using conn_space::receiver;
			
			template<Level l>
			class SQLReceiver{
			public:
				
				receiver r;

				using action_t = typename receiver::action_t;
				using sizes_t = std::vector<std::size_t>;

				static action_t new_connection(){
					struct ReceiverFun : public conn_space::ReceiverFun{
						std::unique_ptr<LocalSQLConnection<l> > db_connection{
							new LocalSQLConnection<l>()};
						std::unique_ptr<LocalSQLTransaction<l> > current_trans{nullptr};
						void operator()(const void* data, mutils::connection& conn){
							const char* _data = (const char*) data;
							if (!current_trans) {
								
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
								}
							}
							else if (_data[0] == 0){
								//we're finishing this transaction
								db_connection = current_trans->store_commit(std::move(current_trans),conn);
								assert(db_connection);
							}
							else if (_data[0] == 1){
								//we're aborting this transaction
								db_connection = current_trans->store_abort(std::move(current_trans),conn);
								assert(db_connection);
							}
							else {
								assert(_data[0] != 4);
								assert(_data[0] == 3);
								TransactionNames name = *((TransactionNames*) (_data + 1));
								auto pair = current_trans->receiveSQLCommand(
									std::move(current_trans),
									name, _data + 1 + sizeof(name),
									conn
									);
								current_trans = std::move(pair.first);
								db_connection = std::move(pair.second);
								assert(current_trans || db_connection);
							}

							/*
							if (current_trans){
								current_trans->log_file << "done processing this request" << std::endl;
								current_trans->log_file.flush();
							}
							else {
								log_file << "done processing this request; transaction was destroyed"
										 << std::endl;
							}//*/
						}
						ReceiverFun(ReceiverFun&& o)
							:db_connection(std::move(o.db_connection)),
							 current_trans(std::move(o.current_trans))
							{}
						ReceiverFun() = default;
					};
					return action_t{new ReceiverFun()};
				}
				
				SQLReceiver():r((l == Level::strong?
								 strong_sql_port :
								 causal_sql_port),
				 				new_connection){}
			};
		}
	}
}
