#pragma once
#include "SQLConnection.hpp"
#include "Basics.hpp"
#include "postgresql/libpq-fe.h"
#include "oid_translator.hpp"
#include <endian.h>

namespace myria { namespace pgsql {
		namespace local{
			
			class LocalSQLConnection_super;
			
			/*throws failure condition on error */
			void check_error(LocalSQLConnection_super &conn, const std::string& command, int result);

		class LocalSQLConnection_super {
		public:

			struct result {
				const std::string command;
				LocalSQLConnection_super& conn;
				PGresult * res;
				~result();
				result(const result&) = delete;
				result(result&&);
				result(const std::string& command,
					   LocalSQLConnection_super &conn, PGresult *res);
			};
			
			using sizes_t = std::vector<std::size_t>;
			
			std::vector<bool> prepared;

			struct deferred_action{
				const std::function<void (result)> on_complete;
				const std::string query_str;
				const std::function<void ()> query;
				deferred_action(
					std::function<void (result)> on_complete,
					const std::string &query_str,
					std::function<void ()> query)
					:on_complete(on_complete),
					 query_str(query_str),
					 query(query){}
				bool submitted{false};
				deferred_action(const deferred_action&) = delete;
			};

			struct transaction;

			struct deferred_transaction{
			private:
				bool no_fut_actions{false};
			public:
				bool no_future_actions() const {
					return no_fut_actions;
				}
				
				void indicate_no_future_actions(){
					if (!no_fut_actions){
						if (trans){
							trans->indicate_no_future_actions();
						}
						else no_fut_actions = true;
					}
				}
				transaction* trans;
				std::list<deferred_action> actions;
				deferred_transaction(transaction& trans)
					:trans(&trans){}

				~deferred_transaction(){
					if (trans){
						trans->my_trans = nullptr;
					}
				}

				friend struct transaction;
			};
			
			std::list<deferred_transaction> transactions;
			
			PGconn *conn;
			std::shared_ptr<bool> aborting{new bool{false}};
			LocalSQLConnection_super();

			LocalSQLConnection_super(const LocalSQLConnection_super&) = delete;
			
			template<typename... Types>
			void prepare(const std::string &name, const std::string &statement){
				Oid oids[] = {PGSQLinfo<Types>::value...};
				result{statement,*this,PQprepare(conn,name.c_str(),statement.c_str(),sizeof...(Types),oids)};
			}

			struct transaction {
			private:
				bool no_fut_actions{false};
			public:

				bool no_future_actions() const {
					return no_fut_actions;
				}

				void indicate_no_future_actions(){
					if (!no_fut_actions){
						assert(my_trans);
						assert(!my_trans->no_fut_actions);
						no_fut_actions = true;
						my_trans->no_fut_actions = true;
					}
				}
				
				const std::size_t transaction_id;
				LocalSQLConnection_super &conn;
				deferred_transaction* my_trans;
				transaction(LocalSQLConnection_super &conn, const std::size_t tid);

				result exec_sync (const std::string &command);

				void commit(std::function<void ()> action);

				void abort(std::function<void ()> action);

			private:
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
				std::function<void (result)> prep_return_func(std::function<void (Args...)> action ){
					return [action](result r){
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
			public:

				template<typename F>
				void exec_async(F action, const std::string &command){
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
				void exec_prepared(F action, const std::string& name, const Args & ... args){
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
				
				transaction(const transaction&) = delete;

				~transaction(){
					if (!conn.aborting){
						assert(no_future_actions());
						if (!no_future_actions()) abort([]{});
					}
					if (my_trans){
						my_trans->trans = nullptr;
					}
				}
			};

			void tick();
			
			int underlying_fd();

			virtual ~LocalSQLConnection_super();
			
		};
			
			struct SerializationFailure : public std::exception{
				const std::string why;
				std::shared_ptr<bool> aborting;
				SerializationFailure(LocalSQLConnection_super& conn,
									 const std::string& command_str,
									 const std::string& err_str)
					:why(std::string("When executing command: ")
						 + command_str
						 + std::string(" we encountered the serialization-related error ")
						 + err_str),
					 aborting(conn.aborting){
					*aborting = true;
				}
				~SerializationFailure(){
					*aborting = false;
				}
				const char* what() const noexcept {
					return why.c_str();
				}
			};

			struct SQLFailure : public std::exception {
				const std::string why;
				std::shared_ptr<bool> aborting;
				SQLFailure(LocalSQLConnection_super& conn,
						   const std::string& command_str,
						   const std::string& err_str)
					:why(std::string("When executing command: ")
						 + command_str
						 + std::string(" we encountered the error ") + err_str),
					 aborting(conn.aborting){
					assert(!*aborting);
					*aborting = true;
				}
				~SQLFailure(){
					*aborting = false;
				}
				const char* what() const noexcept {
					return why.c_str();
				}
			};

			std::ostream & operator<<(std::ostream &os, const LocalSQLConnection_super::result& t);
			template<Level l> struct LocalSQLConnection : public LocalSQLConnection_super {};
		}
	}
}
