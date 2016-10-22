#pragma once
#include "SQLConstants.hpp"
#include <pqxx/result>

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

				LocalSQLTransaction_super(pqxx::connection &conn);

				virtual ~LocalSQLTransaction_super(){
					assert(aborted_or_committed);
				}

				template<typename Conn, typename T>
				void sendBack(Conn &conn, const T& t);

				template<typename E>
				auto exec_prepared_hlpr(E &e);
				
				template<typename E, typename A, typename... B>
				auto exec_prepared_hlpr(E &e, A&& a, B && ... b);
				
				template<typename SQL_Conn, typename Arg1, typename... Args>
				auto prepared(SQL_Conn& sql_conn, LocalTransactionNames name, const std::string &stmt,
							  Arg1 && a1, Args && ... args);

				void store_commit() {
					try{
						trans.commit();
						aborted_or_committed = true;
						all_fine();
					}
					catch(const pqxx::pqxx_exception& e){
						assert(is_serialize_error(e));
						indicate_serialization_failure();
						store_abort();
					}
				}

				virtual void store_abort() = 0;

				//Commands:
				mutils::DeserializationManager dsm{{}};
				virtual void select_version(char const * const bytes) = 0;
				virtual void select_version_data(char const * const bytes) = 0;
				virtual void update_data(char const * const bytes) = 0;
				virtual void initialize_with_id(char const * const bytes) = 0;
				virtual void increment(char const * const bytes) = 0;
				
				virtual void obj_exists(Name id) = 0;
				
				virtual void remove(Name id) = 0;

				virtual void indicate_serialization_failure() = 0;
				virtual void all_fine() = 0;
				
				void receiveSQLCommand(TransactionNames name, char const * const bytes){
					using namespace mutils;
					try{
						switch(name){
						case TransactionNames::exists:
							obj_exists(*from_bytes<Name>(&dsm,bytes));
							break;
						case TransactionNames::Del:
							remove(*from_bytes<Name>(&dsm,bytes));
							break;
						case TransactionNames::select_version:
							select_version(bytes);
							break;
						case TransactionNames::select_version_data:
							select_version_data(bytes);
							break;
						case TransactionNames::update_data:
							update_data(bytes);
							break;
						case TransactionNames::initialize_with_id:
							initialize_with_id(bytes);
							break;
						case TransactionNames::increment:
							increment(bytes);
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
						indicate_serialization_failure();
						store_abort();
					}
				}
			};

			template<Level> class LocalSQLTransaction;
			
			template<>
			class LocalSQLTransaction<Level::strong> : public LocalSQLTransaction_super{
			public:
				static constexpr Level l = Level::strong;
				
				LocalSQLConnection<l> &conn;

				LocalSQLTransaction(LocalSQLConnection<l> &conn);
				
				auto select_version_s(Table t, Name id);
				
				auto select_version_data_s(Table t, Name id);
				
				auto update_data_s(Table t, Name id, char const * const b);
				
				auto increment_s(Table t, Name id);				

				void initialize_with_id_s(Table t, Name id, char const * const b);
				
				auto extract_version(const pqxx::result &r);

				void obj_exists(Name id);
				
				void remove(Name id);
				
				void select_version(char const * const bytes);
				
				void select_version_data(char const * const bytes);
				
				void update_data(char const * const bytes);
				
				void initialize_with_id(char const * const bytes);
				
				void increment(char const * const bytes);

				void store_abort();

				void indicate_serialization_failure();
				void all_fine();
			};
			
			template<>
			class LocalSQLTransaction<Level::causal> : public LocalSQLTransaction_super{
			public:
				static constexpr Level l = Level::causal;

				LocalSQLConnection<l> &conn;
				
				LocalSQLTransaction(LocalSQLConnection<l> &conn);
				
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

				void obj_exists(Name id);
				
				void remove(Name id);
				
				void select_version(char const * const bytes);
				
				void select_version_data(char const * const bytes);
				void update_data(char const * const bytes);
				void initialize_with_id(char const * const bytes);
				void increment(char const * const bytes);

				void store_abort();

				void indicate_serialization_failure();
				void all_fine();
			};
		}
	}
}
