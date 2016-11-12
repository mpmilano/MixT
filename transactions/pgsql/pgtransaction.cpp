#include "pgtransaction_impl.hpp"
#include "pgresult.hpp"

namespace myria { namespace pgsql {
		namespace local{

			pgtransaction::pgtransaction(LocalSQLConnection_super &conn, const std::size_t tid)
				:transaction_id(tid),
				 conn(conn),
				 my_trans((conn.transactions.emplace_back(*this), &conn.transactions.back()))
			{
#ifndef NDEBUG
				for (auto &trans : conn.transactions){
					assert(trans.no_future_actions() || &trans == my_trans);
				}
#endif
				exec_async<std::function<void()> >([]{},"BEGIN");
			}
			
			bool pgtransaction::no_future_actions() const {
					return no_fut_actions;
			}

			void pgtransaction::indicate_no_future_actions(){
				assert(my_trans);
					if (my_trans){
						assert(my_trans);
						assert(!my_trans->no_fut_actions);
						no_fut_actions = true;
						my_trans->no_fut_actions = true;
					}
				}

			/*
			pgresult pgtransaction::exec_sync (const std::string &command) {
				assert(!no_future_actions());
				return pgresult{command,conn,PQexec(conn.conn,command.c_str())};
				}//*/

			void pgtransaction::commit(std::function<void ()> action){
				assert(!no_future_actions());
				exec_async(action, "END");
				indicate_no_future_actions();
			}
			
			void pgtransaction::abort(std::function<void ()> action){
				assert(!no_future_actions());
				exec_async(action, "ABORT");
				exec_async<std::function<void ()> >([]{}, "END");
				indicate_no_future_actions();
			}
			
			pgtransaction::~pgtransaction(){
				if (!conn.aborting){
					assert(no_future_actions());
					if (!no_future_actions()) abort([]{});
					}
				if (my_trans){
					my_trans->trans = nullptr;
				}
			}
		}}}
