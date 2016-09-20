#pragma once
#include "LocalSQLConnection.hpp"
#include "LocalSQLTransaction.hpp"
#include "SQLConstants.hpp"

namespace myria {
	namespace pgsql {
		namespace local {
			template<Level l>
			class SQLReceiver{
				mutils::batched_connection::receiver r;
			public:

				using action_t = typename mutils::batched_connection::receiver::action_t;
				using sizes_t = std::vector<std::size_t>;

				static std::function<void (void*, mutils::connection&)> new_connection(){
					return [db_connection = std::make_shared<LocalSQLConnection<l> >()] 
						(void* data, mutils::connection& conn) {
						char* _data = (char*) data;
						if (!db_connection->current_trans) {
							assert(_data[0] == 4);
							LocalSQLConnection<l> &conn_test = *db_connection;
							db_connection->current_trans
								= std::make_unique<LocalSQLTransaction<l> >(conn_test);
							db_connection->client_connection = &conn;
						}
						else if (_data[0] == 0){
							//we're finishing this transaction
							auto trans = std::move(db_connection->current_trans);
							trans->store_commit();
							db_connection->client_connection = nullptr;
						}
						else if (_data[0] == 1){
							//we're aborting this transaction
							auto trans = std::move(db_connection->current_trans);
							db_connection->client_connection = nullptr;
							trans->aborted_or_committed = true;
							//there is no explicit abort in pqxx we need to worry about
						}
						else if (_data[0] == 2){
							db_connection->current_trans->exec(_data + 1);
						}
						else {
							assert(_data[0] != 4);
							TransactionNames name = *((TransactionNames*) (_data + 1));
							return db_connection->current_trans->receiveSQLCommand(
								name, _data + 1 + sizeof(name));
						}
						
					};
				}
				
				SQLReceiver():r((l == Level::strong?
								 strong_sql_port :
								 causal_sql_port),
								new_connection){}
			};
		}
	}
}
