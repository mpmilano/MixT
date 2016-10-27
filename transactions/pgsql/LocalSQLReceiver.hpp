#pragma once
#include "LocalSQLConnection.hpp"
#include "LocalSQLTransaction.hpp"
#include "SQLConstants.hpp"
#include "simple_rpc.hpp"

namespace myria {
	namespace pgsql {
		namespace local {

			using mutils::batched_connection::receiver;
			
			template<Level l>
			class SQLReceiver{
			public:
				
				receiver r;

				using action_t = typename receiver::action_t;
				using sizes_t = std::vector<std::size_t>;

				static action_t new_connection(){
					struct ReceiverFun : public mutils::batched_connection::ReceiverFun{
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
								//if we're aborting a non-existant transaction, there's nothing to do.
								if (_data[0] == 4){
									assert(!current_trans);
									current_trans.reset(new LocalSQLTransaction<l>(
															std::move(db_connection)));
								}
							}
							else if (_data[0] == 0){
								//we're finishing this transaction
								current_trans->store_commit(std::move(current_trans),conn);
							}
							else if (_data[0] == 1){
								//we're aborting this transaction
								current_trans->store_abort(std::move(current_trans),conn);
							}
							else {
								assert(_data[0] != 4);
								assert(_data[0] == 3);
								TransactionNames name = *((TransactionNames*) (_data + 1));
								current_trans = current_trans->receiveSQLCommand(
									std::move(current_trans),
									name, _data + 1 + sizeof(name),
									conn
									);
							}
							
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
