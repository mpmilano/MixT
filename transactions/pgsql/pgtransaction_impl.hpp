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
			std::function<void (pgresult)> prep_return_func(std::function<void (Args...)> action ){
				return [action](pgresult r){
#ifndef NDEBUG
					if (sizeof...(Args) > 0){
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
					prep_return_func(action),
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
				shared_ptr<vector<char> > scratch_buf{new vector<char>()};
				const vector<std::size_t> indices{{PGSQLinfo<Args>::pg_data_index(*scratch_buf,args)...}};
				vector<const char* > param_values;
				for (const auto& indx : indices){
					param_values.push_back(&(*scratch_buf)[indx]);
				}
				assert(scratch_buf->size() >= (PGSQLinfo<Args>::pg_size(*scratch_buf,args) + ... + 0));
				const vector<int> param_lengths {{static_cast<int>(PGSQLinfo<Args>::pg_size(*scratch_buf,args))...}};
				const vector<int> param_formats{{one<Args>::value...}};
				int result_format = 1;
				auto &conn = this->conn;
				auto transaction_id = this->transaction_id;
				my_trans->actions.emplace_back(
					prep_return_func(action),
					name,
					[&conn, scratch_buf,name,
					 param_values,param_lengths,param_formats,result_format,transaction_id]{
					  check_error(transaction_id,conn,
									name,
									PQsendQueryPrepared(
										conn.conn,name.c_str(),
										sizeof...(Args),
										param_values.data(),
										param_lengths.data(),
										param_formats.data(),
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
