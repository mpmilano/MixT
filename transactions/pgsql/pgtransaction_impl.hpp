#pragma once
#include "pgtransaction.hpp"
#include "pgresult.hpp"
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
					if (sizeof...(Args) > 0){
						assert(PQntuples(r.res) == 1);
						assert(PQnfields(r.res) == sizeof...(Args));
						assert(PQbinaryTuples(r.res) == 1);
						std::tuple<Args...> tpl = deserialize<0,Args...>(r.res);
						mutils::callFunc(action,tpl);
					}
					else {
						//this code needs to be here for cases when this branch doesn't match.
						//whenever this branch matches the tuple will be empty.
						std::tuple<Args...> tpl = deserialize<0,Args...>(nullptr);
						mutils::callFunc(action,tpl);
					}
				};
			}

			template<typename F>
			void pgtransaction::exec_async(F action, const std::string &command){
				assert(!no_future_actions());
				assert(my_trans);
				my_trans->actions.emplace_back(
					prep_return_func(action),
					command,
						[=]{
						check_error(conn,command,PQsendQueryParams(
										conn.conn,
										command.c_str(),
										0,nullptr,nullptr,nullptr,nullptr,
										1));
					}
					);
			}

			template<typename F, typename... Args>
			void pgtransaction::exec_prepared(F action, const std::string& name, const Args & ... args){
				assert(!no_future_actions());
				assert(my_trans);
				using namespace std;
				shared_ptr<vector<char> > scratch_buf{new vector<char>};
				const vector<const char* > param_values{{PGSQLinfo<Args>::pg_data(*scratch_buf,args)...}};
				const vector<int> param_lengths {{PGSQLinfo<Args>::pg_size(*scratch_buf,args)...}};
				const vector<int> param_formats{{one<Args>::value...}};
				int result_format = 1;
				auto &conn = this->conn;
				my_trans->actions.emplace_back(
					prep_return_func(action),
					name,
					[&conn, sb = std::move(scratch_buf),name,
					 param_values,param_lengths,param_formats,result_format]{
						check_error(conn,
									name,
									PQsendQueryPrepared(
										conn.conn,name.c_str(),
										sizeof...(Args),
										param_values.data(),
										param_lengths.data(),
										param_formats.data(),
										result_format));});
			}
			
		}}}
