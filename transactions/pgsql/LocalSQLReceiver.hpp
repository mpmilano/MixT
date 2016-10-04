#pragma once
#include "LocalSQLConnection.hpp"
#include "LocalSQLTransaction.hpp"
#include "SQLConstants.hpp"
#include "simple_rpc.hpp"

namespace myria {
	namespace pgsql {
		namespace local {

			using mutils::proxy_connection::receiver;
			
			template<Level l>
			class SQLReceiver{
			public:
				
				receiver r;

				using action_t = typename receiver::action_t;
				using sizes_t = std::vector<std::size_t>;

				static std::function<void (const void*, mutils::connection&)> new_connection(){
					return [db_connection = std::make_shared<LocalSQLConnection<l> >()] 
						(const void* data, mutils::connection& conn) {
						const char* _data = (const char*) data;
						if (!db_connection->current_trans) {
							if (_data[0] != 4 && _data[0] != 1){
								std::cout << (int) _data[0] << std::endl;
							}
							assert(_data[0] == 4 || _data[0] == 1);
							//if we're aborting a non-existant transaction, there's nothing to do.
							if (_data[0] == 4){
								LocalSQLConnection<l> &conn_test = *db_connection;
								db_connection->current_trans
									= std::make_unique<LocalSQLTransaction<l> >(conn_test);
								db_connection->client_connection = &conn;
							}
						}
						else if (_data[0] == 0){
							//we're finishing this transaction
							auto trans = std::move(db_connection->current_trans);
							trans->store_commit();
							db_connection->client_connection = nullptr;
						}
						else if (_data[0] == 1){
							//we're aborting this transaction
							db_connection->current_trans->store_abort();
						}
						else if (_data[0] == 2){
							assert(false);
							//db_connection->current_trans->exec(*from_bytes<std::string>(nullptr,_data + 1));
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
