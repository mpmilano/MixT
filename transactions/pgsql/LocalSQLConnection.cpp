#include "LocalSQLConnection.hpp"
#include <stdio.h>

using namespace mutils;

namespace myria { namespace pgsql {
		namespace local{
			LocalSQLConnection_super::result::~result(){
				if (res) PQclear(res);
			}
			
			LocalSQLConnection_super::result::result(result&& r)
				:command(std::move(r.command)),
				 conn(r.conn),
				 res(r.res){
				r.res = nullptr;
			}
			
			LocalSQLConnection_super::result::result(const std::string& command,
													 LocalSQLConnection_super &conn,
													 PGresult *res)
				:command(command),
				 conn(conn),
				 res(res)
			{
				auto status = PQresultStatus(res);
				if (status == PGRES_EMPTY_QUERY
					|| status == PGRES_COMMAND_OK
					|| status == PGRES_TUPLES_OK
					|| status == PGRES_SINGLE_TUPLE
					|| status == PGRES_COPY_OUT
					|| status == PGRES_COPY_BOTH
					|| status == PGRES_COPY_IN){
#ifndef NDEBUG
					std::cout << "result successfully processed: status was :" <<
						PQresStatus(status) << std::endl;
#endif
				}
				else if (status == PGRES_BAD_RESPONSE
						 || status == PGRES_FATAL_ERROR
						 || status == PGRES_NONFATAL_ERROR){
					if (auto _errorcode = PQresultErrorField(res,PG_DIAG_SQLSTATE)){
						std::string errorcode{_errorcode};
						if (errorcode == "40000"
							|| errorcode == "40002"
							|| errorcode == "40001"
							|| errorcode == "40003"
							|| errorcode == "40P01"){
							throw SerializationFailure{
								conn,
									command,
									PQresultErrorMessage(res)};
						}
					}
					throw SQLFailure{conn,command,PQresultErrorMessage(res)};
				}
			}

			void check_error(LocalSQLConnection_super &conn,
							 const std::string &command,
							 int result){
				if (!result){
					throw SQLFailure{conn,command,PQerrorMessage(conn.conn)};
				}
			}

			std::ostream & operator<<(std::ostream &os, const LocalSQLConnection_super::result&){
				return os << "(look we just can't print this)";
			}

			LocalSQLConnection_super::LocalSQLConnection_super()
				:prepared(((std::size_t) LocalTransactionNames::MAX),false),
				 conn(PQconnectdb(""))
			{
				assert(conn);
			}

			LocalSQLConnection_super::transaction::transaction(LocalSQLConnection_super &conn)
				:conn(conn) {
				exec_async<std::function<void()> >([]{},"BEGIN");
			}
			LocalSQLConnection_super::result LocalSQLConnection_super::transaction::exec_sync (const std::string &command) {
				return result{command,conn,PQexec(conn.conn,command.c_str())};
			}

			void LocalSQLConnection_super::transaction::commit(std::function<void ()> action){
				exec_async(action, "END");
				sent_destroy = true;
			}
			
			void LocalSQLConnection_super::transaction::abort(std::function<void ()> action){
				exec_async(action, "ABORT");
				exec_async<std::function<void ()> >([]{}, "END");
				sent_destroy = true;
			}

			void LocalSQLConnection_super::tick(){
				if (actions.size() > 0){
					auto &front = actions.front();
					if (!front.submitted){
						front.submitted = true;
						front.query();
					}
				}
				PQconsumeInput(conn);
				while (!PQisBusy(conn)){
					if (auto* res = PQgetResult(conn)){
						auto &action = actions.front();
						AtScopeEnd ase{[&]{actions.pop_front();}};
						assert(action.submitted);
						std::cout << "executing response evaluator for " << action.query_str << std::endl;
						action.on_complete(result{action.query_str,*this,res});
					}
					else break;
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
