#pragma once
#include "pgtransaction.hpp"
#include "pgresult.hpp"
#include "postgresql/libpq-fe.h"
namespace myria { namespace pgsql {
		namespace local{
			
			template<int>
			static std::tuple<> deserialize(...){
				return std::tuple<>();
			}
			
			template<int index, typename Arg1, typename... Args>
			static std::enable_if_t<std::is_same<Arg1, mutils::Bytes>::value, std::tuple<Arg1, Args...> > deserialize(PGresult* res){
				using namespace mutils;
				return std::tuple_cat(
					std::tuple<Bytes>{PGSQLinfo<Arg1>::from_pg(
								PQgetlength(res,0,index),
								PQgetvalue(res,0,index))},
					deserialize<index+1, Args...>(res)
					);
			}
			
			template<int index, typename Arg1, typename... Args>
			static std::enable_if_t<std::is_integral<Arg1>::value, std::tuple<Arg1, Args...> > deserialize(PGresult* res){
				assert(PQgetlength(res,0,index) == sizeof(Arg1));
				return std::tuple_cat(
					std::tuple<Arg1>{PGSQLinfo<Arg1>::from_pg(PQgetvalue(res,0,index))},
					deserialize<index+1, Args...>(res)
					);
			}
			
			template<typename... Args>
			std::function<void (pgresult)> prep_return_func(whendebug(const std::string& command, ) std::function<void (Args...)> action ){
				return [=](pgresult r){
#ifndef NDEBUG
					if (sizeof...(Args) > 0){
						if (!(PQntuples(r.res) == 1 && PQnfields(r.res) == sizeof...(Args) && PQbinaryTuples(r.res) == 1)){
							std::cerr << "Error: command was " << command << std::endl;
						}
						assert(PQntuples(r.res) == 1);
						assert(PQnfields(r.res) == sizeof...(Args));
						assert(PQbinaryTuples(r.res) == 1);
					}
#endif
					PGresult* ds_ptr = (sizeof...(Args) > 0 ? r.res : nullptr);
					std::tuple<Args...> tpl = deserialize<0,Args...>(ds_ptr);
					mutils::callFunc(action,tpl);
				};
			}

			template<typename F>
			void pgtransaction::exec_async(F action, const std::string &command){
				assert(!no_future_actions());
				assert(my_trans);
				auto* pgconn = conn.conn;
				auto &conn = this->conn;
				auto transaction_id = this->transaction_id;
				my_trans->actions.emplace_back(
					prep_return_func(whendebug(command, ) action),
					command,
					[&conn,command,pgconn,transaction_id]{
					  check_error(transaction_id,conn,command,PQsendQueryParams(
										pgconn,
										command.c_str(),
										0,nullptr,nullptr,nullptr,nullptr,
										1));
					}
					);
				conn.tick();
			}

			template<typename F, typename... Args>
			void pgtransaction::exec_prepared(F action, const std::string& name, const Args & ... args){
				assert(!no_future_actions());
				assert(my_trans);
				using namespace std;
				auto &conn = this->conn;
				auto transaction_id = this->transaction_id;
				my_trans->actions.emplace_back(
					prep_return_func(whendebug(name, )action),
					name,
					[&conn,name,transaction_id, info = std::make_shared<PGSQLArgsHolder<Args...> >(args...)]{
						constexpr int result_format = 1;
					  check_error(transaction_id,conn,
												name,
												PQsendQueryPrepared(
													conn.conn,name.c_str(),
													sizeof...(Args),
													info->param_values.data(),
													info->param_lengths.data(),
													info->param_formats.data(),
													result_format));});
				conn.tick();
			}

			template<typename... Types>
			void pgtransaction::prepare(const std::string &name, const std::string &statement){
				auto &conn = this->conn;
				auto transaction_id = this->transaction_id;
				my_trans->actions.emplace_back(
					[](pgresult){},name + statement,
					[&conn,name,statement,transaction_id]{
						Oid oids[] = {PGSQLinfo<Types>::value...};
						check_error(transaction_id,
							conn,
							name + statement,
							PQsendPrepare(conn.conn,name.c_str(),statement.c_str(),sizeof...(Types),oids));
					}
					);
				conn.tick();
			}
		}}}

#include "LocalSQLConnection_subcls.hpp"
