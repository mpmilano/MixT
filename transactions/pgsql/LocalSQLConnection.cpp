#include "LocalSQLConnection.hpp"
#include "pgtransaction.hpp"
#include "pgexceptions.hpp"
#include "pgresult.hpp"
#include <stdio.h>

using namespace mutils;

namespace myria { namespace pgsql {
		namespace local{
			void check_error(LocalSQLConnection_super &conn,
							 const std::string &command,
							 int result){
				if (!result){
					throw SQLFailure{conn,command,PQerrorMessage(conn.conn)};
				}
			}

			LocalSQLConnection_super::LocalSQLConnection_super()
				:prepared(((std::size_t) LocalTransactionNames::MAX),false),
				 conn(PQconnectdb(""))
			{
				assert(conn);
			}

			namespace {
				template<typename transaction_list>
				void clear_completed_transactions(transaction_list &transactions){
					while (true){
						auto &front = transactions.front();
						if (front.actions.size() == 0 && front.no_future_actions()) {
							transactions.pop_front();
						}
						else break;
					}
				}
			}

			void LocalSQLConnection_super::tick(){
				clear_completed_transactions(transactions);
				try {
					if (transactions.size() > 0){
						auto &front = transactions.front();
						if (front.actions.size() > 0){
							auto &action = front.actions.front();
							if (!action.submitted){
								action.submitted = true;
								action.query();
							}
						}
					}
					PQconsumeInput(conn);
					while (!PQisBusy(conn)){
						if (auto* res = PQgetResult(conn)){
							auto &trans = transactions.front();
							auto &actions = trans.actions;
							auto &action = actions.front();
							AtScopeEnd ase{[&]{actions.pop_front();}};
							assert(action.submitted);
							std::cout << "executing response evaluator for " << action.query_str << std::endl;
							action.on_complete(pgresult{action.query_str,*this,res});
							clear_completed_transactions(transactions);
						}
						else break;
					}
				}
				catch (SerializationFailure& sf){
					transactions.front().indicate_no_future_actions();
					transactions.pop_front();
					throw sf;
				}
			}
			
			int LocalSQLConnection_super::underlying_fd() {
				return PQsocket(conn);
			}
			LocalSQLConnection_super::~LocalSQLConnection_super(){
				PQfinish(conn);
			}
			
		}
	}
}
