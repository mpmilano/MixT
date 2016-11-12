#include "LocalSQLConnection.hpp"
#include "pgtransaction.hpp"
#include "pgexceptions.hpp"
#include "pgresult.hpp"
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

using namespace mutils;

namespace myria { namespace pgsql {
		namespace local{

			const std::function<void ()> noop{[]{}};
			
			void check_error(LocalSQLConnection_super &conn,
							 const std::string &command,
							 int result){
				if (!result){
					assert(false && "command error");
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
					while (transactions.size() > 0){
						auto &front = transactions.front();
						if (front.actions.size() == 0 && front.no_future_actions()) {
							transactions.pop_front();
						}
						else break;
					}
				}
			}

#ifndef NDEBUG
			namespace {
				bool select_indicates_data_avilable(int fd){
					fd_set rfds;
					struct timeval tv;
					int retval;

					/* Watch stdin (fd 0) to see when it has input. */

					FD_ZERO(&rfds);
					FD_SET(fd, &rfds);

					/* Wait up to one second. */

					tv.tv_sec = 1;
					tv.tv_usec = 0;

					retval = select(fd + 1, &rfds, NULL, NULL, &tv);
					/* Don't rely on the value of tv now! */

					if (retval == -1)
						perror("select()");
					else return retval;
					assert(false && "select error");
				}
			}
#endif

			void LocalSQLConnection_super::submit_new_transaction(){
				if (transactions.size() > 0){
					auto &front = transactions.front();
					if (front.actions.size() > 0){
						auto &action = front.actions.front();
						if (!action.submitted){
							action.submitted = true;
							action.query();
							std::cout << "SUBMIT QUERY SUCCESS" << std::endl;
						}
					}
				}
			}
			
			void LocalSQLConnection_super::tick(){
#ifndef NDEBUG
				if (select_indicates_data_avilable(underlying_fd()))
					std::cout << "======SELECT indicates data available" << std::endl;
#endif
				clear_completed_transactions(transactions);

				try {
					submit_new_transaction();
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
					//whendebug(std::cout << "PQ tick successfully completed!" << std::endl);
				}
				catch (SerializationFailure& sf){
					whendebug(std::cerr << "serialization failure detected!" << std::endl);
					transactions.front().indicate_no_future_actions();
					transactions.pop_front();
					throw sf;
				}
				clear_completed_transactions(transactions);
				submit_new_transaction();
			}
			
			int LocalSQLConnection_super::underlying_fd() {
				auto fd = PQsocket(conn);
				assert(fd >= 0);
				return fd;
			}
			LocalSQLConnection_super::~LocalSQLConnection_super(){
				whendebug(std::cout << "closing a postgres connection!" << std::endl;)
				PQfinish(conn);
			}
			
		}
	}
}
