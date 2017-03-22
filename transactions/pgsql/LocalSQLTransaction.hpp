#pragma once
#include "SQLConstants.hpp"
#include "pgtransaction.hpp"
#include <fstream>

namespace myria { namespace pgsql {

		namespace local{

			template<Level> class LocalSQLConnection;
			
			class LocalSQLTransaction_super{
			public:

				pgtransaction trans;
				bool aborted_or_committed{false};

#ifndef NDEBUG
				std::ostream &log_file;

				static void log_receive(std::ostream &log_file, const std::string& s){
					log_file << "received: " << s << std::endl;
					log_file.flush();
				}
				
				static void log_send(std::ostream &log_file, const std::string& s){
					log_file << "sent: " << s << std::endl;
					log_file.flush();
					}
#else
#define log_receive(...) ;
#define log_send(...) ;
#endif

				LocalSQLTransaction_super(LocalSQLConnection_super &conn whendebug(, std::ostream& log_file));

				virtual ~LocalSQLTransaction_super(){
					assert(aborted_or_committed);
					trans.abort(noop);
				}

				template<typename T>
				void sendBack(const std::string&, const T& t, mutils::connection& socket);

				template<typename F, typename Arg1, typename... Args>
				void prepared(const F& f,LocalSQLConnection_super& sql_conn, LocalTransactionNames name, const std::string &stmt,  Arg1 && a1, Args && ... args);

				template<typename LocalSQLTransaction>
				typename std::unique_ptr<typename LocalSQLTransaction::SQLConn>
				store_commit(std::unique_ptr<LocalSQLTransaction> o, mutils::connection& socket) {
					whendebug(auto& log_file = o->log_file);
					log_receive(log_file,"commit");
					o->trans.commit([&]{all_fine(whendebug(log_file,) socket);});
					o->aborted_or_committed = true;
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

				static void indicate_serialization_failure(whendebug(std::ostream& log_file, )  mutils::connection& socket) {
					char abort{1};
					log_send(log_file,"serialization failure");
					socket.send(abort);
				}
				
				static void all_fine(whendebug(std::ostream& log_file, ) mutils::connection& socket) {
					char ok{0};
					whendebug(log_send(log_file,"all fine"));
					socket.send(ok);
				}

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
					switch(name){
					case TransactionNames::exists:
						log_receive(log_file,"exists");
						o->obj_exists(*from_bytes_noalloc<Name>(&o->dsm,bytes),socket);
						break;
					case TransactionNames::Del:
						log_receive(log_file,"del");
						o->remove(*from_bytes_noalloc<Name>(&o->dsm,bytes),socket);
						break;
					case TransactionNames::select_version:
						log_receive(log_file,"select_version");
						o->select_version(bytes,socket);
						break;
					case TransactionNames::select_version_data:
						log_receive(log_file,"select-version-data");
						o->select_version_data(bytes,socket);
						break;
					case TransactionNames::update_data:
						log_receive(log_file,"update-data");
						o->update_data(bytes,socket);
						break;
					case TransactionNames::initialize_with_id:
						log_receive(log_file,"initialize-with-id");
						o->initialize_with_id(bytes,socket);
						break;
					case TransactionNames::increment:
						log_receive(log_file,"increment");
						o->increment(bytes,socket);
						break;
					case TransactionNames::MAX:
						assert(false && "this should not be sent");
						struct dead_code{}; throw dead_code{};
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
				
				LocalSQLTransaction(std::unique_ptr<LocalSQLConnection<l> > conn whendebug(, std::ostream& log_file))
				:LocalSQLTransaction_super(*conn whendebug (,log_file)),
				 conn(std::move(conn)){}
				
				void select_version_s(std::function<void (long int)> action, Table t, Name id);
				
				void select_version_data_s(std::function<void (long int, long int)> action, Table t, Name id);

				void select_version_data_s(std::function<void (long int, mutils::Bytes)> action, Table t, Name id);
				
				void update_data_s(std::function<void (long int)> action, Table t, Name id, char const * const b);
				
				void increment_s(std::function<void (long int)> action, Table t, Name id);

				void initialize_with_id_s(Table t, Name id, char const * const b);

				void obj_exists(Name id, mutils::connection& socket);
				
				void remove(Name id, mutils::connection& socket);
				
				void select_version(char const * const bytes, mutils::connection& socket);
				
				void select_version_data(char const * const bytes,mutils::connection& socket);
				
				void update_data(char const * const bytes,mutils::connection& socket);
				
				void initialize_with_id(char const * const bytes,mutils::connection& socket);
				
				void increment(char const * const bytes,mutils::connection& socket);

				static std::unique_ptr<SQLConn> store_abort(std::unique_ptr<LocalSQLTransaction<Level::strong> >);

				void indicate_serialization_failure(mutils::connection& socket);
				void all_fine(mutils::connection& socket);
			};
			
			template<>
			class LocalSQLTransaction<Level::causal> : public LocalSQLTransaction_super{
			public:
				static constexpr Level l = Level::causal;

				using SQLConn = LocalSQLConnection<l>;
				std::unique_ptr<SQLConn > conn;

				LocalSQLTransaction(std::unique_ptr<LocalSQLConnection<l> > conn whendebug(, std::ostream& log_file))
				:LocalSQLTransaction_super(*conn whendebug(, log_file)),
				 conn(std::move(conn)){}
				
				static constexpr int group_mapper(int k){
					if (k < 1) {
						assert(false && "Error: k is 1-indexed");
						struct dead_code{}; throw dead_code{};
					}
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
						static constexpr CexprString vcs1(unsigned int i, unsigned int stop, unsigned int _k){
							return CexprString{} + ", vc" + md(_k+i)+ "=$"+ (i+1) +
												  ( i==(stop-1) ? CexprString{} : vcs1(i+1,stop,_k));
						}
						
						static constexpr CexprString vcs2(unsigned int i, unsigned int stop){
							return CexprString{} + "vc" + i + "," + (i == (stop-1)? CexprString{} : vcs2(i+1,stop));
						}
						
						pair<LocalTransactionNames,CexprString> ret[Table_max * (NUM_CAUSAL_GROUPS+1)]
							{pair<LocalTransactionNames,CexprString>{LocalTransactionNames::MAX,CexprString{} }};
						
						pair<LocalTransactionNames,string> at(std::size_t s) const {
							return make_pair(ret[s].first,string{ret[s].second.str});
						}
					};
					
					static_assert(NUM_CAUSAL_GROUPS == 4,"This is assumed");
					static_assert(Table_max == 2,"This is assumed");
					
					ReturnThis ret;
					int index{0};
					for (int _t = 0; _t < Table_max; ++_t){
						CexprString t {table_name((Table)_t)};
						for (int _k = 1; _k < (NUM_CAUSAL_GROUPS+1); ++_k){
							auto k = to_cexprstring(_k);
							CexprString main = CexprString{}
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
				
				void select_version_c(std::function<void (long int, long int, long int, long int)> action, Table t, Name id);
				
				void select_version_data_c(std::function<void (long int, long int, long int, long int, long int)> action, Table t, Name id);
					
				void select_version_data_c(std::function<void (long int, long int, long int, long int, mutils::Bytes)> action, Table t, Name id);

				void update_data_c(std::function<void (long int, long int, long int, long int)> action,Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, char const* const b);
				
				void increment_c(std::function<void (long int, long int, long int, long int)> action, Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends);
				
				template<typename Blob>
				void initialize_with_id_c(Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, const Blob &b);

				void obj_exists(Name id,mutils::connection& socket);
				
				void remove(Name id,mutils::connection& socket);
				
				void select_version(char const * const bytes,mutils::connection& socket);
				
				void select_version_data(char const * const bytes,mutils::connection& socket);
				void update_data(char const * const bytes,mutils::connection& socket);
				void initialize_with_id(char const * const bytes,mutils::connection& socket);
				void increment(char const * const bytes,mutils::connection& socket);

				static std::unique_ptr<SQLConn> store_abort(std::unique_ptr<LocalSQLTransaction<Level::causal> >);

				void indicate_serialization_failure(mutils::connection& socket);
				void all_fine(mutils::connection& socket);
			};
		}
	}
}

#include "LocalSQLTransaction_impl.hpp"
