#include "LocalSQLConnection.hpp"
#include "pgtransaction.hpp"
#include "pgexceptions.hpp"
#include "pgresult.hpp"
#include <stdio.h>

using namespace mutils;

namespace myria { namespace pgsql {
		namespace local{

			pgresult::~pgresult(){
				if (res) PQclear(res);
			}
			
			pgresult::pgresult(pgresult&& r)
				:command(std::move(r.command)),
				 conn(r.conn),
				 res(r.res){
				r.res = nullptr;
			}
			
			pgresult::pgresult(const std::string& command,
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
				else {
					assert(false && "odd case");
				}
			}
		}}}
