#pragma once
#include "SQLConstants.hpp"
#include <pqxx/result>
#include <fstream>

namespace myria { namespace pgsql {

		namespace local{

	
			enum class LocalTransactionNames{
				exists, Del1, Del2, select_version_s_i, select_version_s_b,
					select1, select2, Updates1, Updates2, Increment, Insert1, Insert2,
					Sel1i,Sel1b,udc1,udc2,udc3,udc4,udc5,udc6,udc7,udc8,
					ic1,ic2,ic3,ic4,ic5,ic6,ic7,ic8,initci,initcb,
					MAX
					};

			template<Level> class LocalSQLConnection;

			bool is_serialize_error(const pqxx::pqxx_exception &r) {
				auto s = std::string(r.base().what());
				return s.find("could not serialize access") != std::string::npos;
			}
			
			class LocalSQLTransaction_super{
			public:

				pqxx::work trans;
				bool aborted_or_committed{false};
				std::ofstream &log_file;

				void log_receive(const std::string& s){
					log_file << "received: " << s << std::endl;
					log_file.flush();
				}
				
				void log_send(const std::string& s){
					log_file << "sent: " << s << std::endl;
					log_file.flush();
				}

				template<typename SQLConn>
				LocalSQLTransaction_super(SQLConn &conn, std::ofstream& log_file);

				virtual ~LocalSQLTransaction_super(){
					assert(aborted_or_committed);
				}

				template<typename T>
				void sendBack(const std::string&, const T& t, mutils::connection& socket);

				template<typename E>
				auto exec_prepared_hlpr(E &e);
				
				template<typename E, typename A, typename... B>
				auto exec_prepared_hlpr(E &e, A&& a, B && ... b);
				
				template<typename SQL_Conn, typename Arg1, typename... Args>
				auto prepared(SQL_Conn& sql_conn, LocalTransactionNames name, const std::string &stmt,
							  Arg1 && a1, Args && ... args);

				template<typename LocalSQLTransaction>
				typename std::unique_ptr<typename LocalSQLTransaction::SQLConn>
				store_commit(std::unique_ptr<LocalSQLTransaction> o, mutils::connection& socket) {
					log_receive("commit");
					try{
						o->trans.commit();
						o->aborted_or_committed = true;
						o->all_fine(socket);
					}
					catch(const pqxx::pqxx_exception& e){
						assert(is_serialize_error(e));
						o->indicate_serialization_failure(socket);
						return (o->store_abort(std::move(o),socket));
					}
					return std::move(o->conn);
				}

				//Commands:
				mutils::DeserializationManager dsm{{}};
				virtual void select_version(char const * const bytes, mutils::connection& socket) = 0;
				virtual void select_version_data(char const * const bytes, mutils::connection& socket) = 0;
				virtual void update_data(char const * const bytes, mutils::connection& socket) = 0;
				virtual void initialize_with_id(char const * const bytes, mutils::connection& socket) = 0;
				virtual void increment(char const * const bytes, mutils::connection& socket) = 0;
				
				virtual void obj_exists(Name id, mutils::connection& socket) = 0;
				
				virtual void remove(Name id, mutils::connection& socket) = 0;

				virtual void indicate_serialization_failure(mutils::connection& socket) = 0;
				virtual void all_fine(mutils::connection& socket) = 0;

				template<typename LocalSQLTransaction>
				auto receiveSQLCommand(std::unique_ptr<LocalSQLTransaction> o, TransactionNames name, char const * const bytes, mutils::connection& socket){
					using namespace mutils;

					struct resource_return{
						std::unique_ptr<LocalSQLTransaction> first;
						std::unique_ptr<typename LocalSQLTransaction::SQLConn> second;
						resource_return(decltype(first) first, decltype(second) second)
							:first(std::move(first)),second(std::move(second)){}
						resource_return(resource_return&& o)
							:first(std::move(o.first)),second(std::move(o.second)){}
					};
					
					try{
						switch(name){
						case TransactionNames::exists:
							log_receive("exists");
							o->obj_exists(*from_bytes<Name>(&o->dsm,bytes),socket);
							break;
						case TransactionNames::Del:
							log_receive("del");
							o->remove(*from_bytes<Name>(&o->dsm,bytes),socket);
							break;
						case TransactionNames::select_version:
							log_receive("select_version");
							o->select_version(bytes,socket);
							break;
						case TransactionNames::select_version_data:
							log_receive("select-version-data");
							o->select_version_data(bytes,socket);
							break;
						case TransactionNames::update_data:
							log_receive("update-data");
							o->update_data(bytes,socket);
							break;
						case TransactionNames::initialize_with_id:
							log_receive("initialize-with-id");
							o->initialize_with_id(bytes,socket);
							break;
						case TransactionNames::increment:
							log_receive("increment");
							o->increment(bytes,socket);
							break;
						case TransactionNames::MAX:
							assert(false && "this should not be send");
						}
					}
					catch(const pqxx::pqxx_exception& e){
						if (!is_serialize_error(e)){
							std::cout << e.base().what() << std::endl;
							assert(false);
						}
						o->indicate_serialization_failure(socket);
						return resource_return{nullptr,
								o->store_abort(std::move(o),socket)};
					}
					return resource_return{std::move(o),nullptr};
				}
			};

			template<Level> class LocalSQLTransaction;
			
			template<>
			class LocalSQLTransaction<Level::strong> : public LocalSQLTransaction_super{
			public:
				static constexpr Level l = Level::strong;

				using SQLConn = LocalSQLConnection<l>;
				std::unique_ptr<SQLConn > conn;

				LocalSQLTransaction(std::unique_ptr<LocalSQLConnection<l> > conn, std::ofstream& log_file);
				
				auto select_version_s(Table t, Name id);
				
				auto select_version_data_s(Table t, Name id);
				
				auto update_data_s(Table t, Name id, char const * const b);
				
				auto increment_s(Table t, Name id);				

				void initialize_with_id_s(Table t, Name id, char const * const b);
				
				auto extract_version(const pqxx::result &r);

				void obj_exists(Name id,mutils::connection& socket);
				
				void remove(Name id,mutils::connection& socket);
				
				void select_version(char const * const bytes,mutils::connection& socket);
				
				void select_version_data(char const * const bytes,mutils::connection& socket);
				
				void update_data(char const * const bytes,mutils::connection& socket);
				
				void initialize_with_id(char const * const bytes,mutils::connection& socket);
				
				void increment(char const * const bytes,mutils::connection& socket);

				static std::unique_ptr<SQLConn> store_abort(std::unique_ptr<LocalSQLTransaction<Level::strong> >,mutils::connection& socket);

				void indicate_serialization_failure(mutils::connection& socket);
				void all_fine(mutils::connection& socket);
			};
			
			template<>
			class LocalSQLTransaction<Level::causal> : public LocalSQLTransaction_super{
			public:
				static constexpr Level l = Level::causal;

				using SQLConn = LocalSQLConnection<l>;
				std::unique_ptr<SQLConn > conn;
				
				LocalSQLTransaction(std::unique_ptr<LocalSQLConnection<l> > conn, std::ofstream& log_file);
				
				static constexpr int group_mapper(int k){
					if (k < 1) assert(false && "Error: k is 1-indexed");
					constexpr int sizes = NUM_CAUSAL_GROUPS / NUM_CAUSAL_MASTERS;
					constexpr int overflow = NUM_CAUSAL_GROUPS - sizes * NUM_CAUSAL_MASTERS;
					return (k < overflow ? 0
							: (k - overflow - 1) / (NUM_CAUSAL_MASTERS)) + 1;
				}
				
				static constexpr int md(int k){
					return (k % NUM_CAUSAL_GROUPS == 0 ? NUM_CAUSAL_GROUPS : k % NUM_CAUSAL_GROUPS);
				}
				
				static constexpr auto update_data_c_cmd(LocalTransactionNames const (&xx)[Table_max * NUM_CAUSAL_GROUPS],
												 char const * const set) {
					using namespace std;
					using namespace mutils;
					struct ReturnThis{
						static constexpr CTString vcs1(unsigned int i, unsigned int stop, unsigned int _k){
							return CTString{} + ", vc" + md(_k+i)+ "=$"+ (i+1) +
												  ( i==(stop-1) ? CTString{} : vcs1(i+1,stop,_k));
						}
						
						static constexpr CTString vcs2(unsigned int i, unsigned int stop){
							return CTString{} + "vc" + i + "," + (i == (stop-1)? CTString{} : vcs2(i+1,stop));
						}
						
						pair<LocalTransactionNames,CTString> ret[Table_max * (NUM_CAUSAL_GROUPS+1)]
							{pair<LocalTransactionNames,CTString>{LocalTransactionNames::MAX,CTString{} }};
						
						pair<LocalTransactionNames,string> at(std::size_t s) const {
							return make_pair(ret[s].first,string{ret[s].second.str});
						}
					};
					
					static_assert(NUM_CAUSAL_GROUPS == 4,"This is assumed");
					static_assert(Table_max == 2,"This is assumed");
					
					ReturnThis ret;
					int index{0};
					for (int _t = 0; _t < Table_max; ++_t){
						CTString t {table_name((Table)_t)};
						for (int _k = 1; _k < (NUM_CAUSAL_GROUPS+1); ++_k){
							auto k = to_ctstring(_k);
							CTString main = CTString{}
							+ "update " + t + " set data = " + set
								+ ReturnThis::vcs1(1,NUM_CAUSAL_GROUPS,_k)
								+ ", lw = " + k
								+ " where id = $1 and index = " + group_mapper(_k)
								+ " returning " + ReturnThis::vcs2(1,NUM_CAUSAL_GROUPS)
								+ "vc" + NUM_CAUSAL_GROUPS;
							ret.ret[index].first = xx[_k + NUM_CAUSAL_GROUPS*_t -1 ];
							ret.ret[index].second = main;
							++index;
						}
					}
					return ret;
				}
				
				auto select_version_c(Table t, Name id);
				
				auto select_version_data_c(Table t, Name id);

				auto update_data_c(Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, char const* const b);
				
				auto increment_c(Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends);
				
				template<typename Blob>
				void initialize_with_id_c(Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, const Blob& b);

				auto extract_version(const pqxx::result &r);

				void obj_exists(Name id,mutils::connection& socket);
				
				void remove(Name id,mutils::connection& socket);
				
				void select_version(char const * const bytes,mutils::connection& socket);
				
				void select_version_data(char const * const bytes,mutils::connection& socket);
				void update_data(char const * const bytes,mutils::connection& socket);
				void initialize_with_id(char const * const bytes,mutils::connection& socket);
				void increment(char const * const bytes,mutils::connection& socket);

				static std::unique_ptr<SQLConn> store_abort(std::unique_ptr<LocalSQLTransaction<Level::causal> >,mutils::connection& socket);

				void indicate_serialization_failure(mutils::connection& socket);
				void all_fine(mutils::connection& socket);
			};
		}
	}
}
