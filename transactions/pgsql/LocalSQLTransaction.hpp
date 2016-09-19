#pragma once
#include "SQLConnection.hpp"

namespace myria { namespace pgsql {

		namespace local{
		
			enum class LocalTransactionNames{
				exists, Del1, Del2, select_version_s_i, select_version_s_b,
					select1, select2, Updates1, Updates2, Increment, Insert1, Insert2,
					Sel1i,Sel1b,udc1,udc2,udc3,udc4,udc5,udc6,udc7,udc8,
					ic1,ic2,ic3,ic4,ic5,ic6,ic7,ic8,initci,initcb,
					MAX
					};
			
			class LocalSQLTransaction_super{

				LocalSQLConnection &conn;
				pqxx::work trans;
				bool aborted_or_committed{false};

				~LocalSQLTransaction_super(){
					assert(aborted_or_committed);
				}

				template<typename T>
				void sendBack(const T& t){
					conn->client_connection->send(t);
				}

				template<typename E>
				auto exec_prepared_hlpr(E &e){
					return e.exec();
				}
				
				template<typename E, typename A, typename... B>
				auto exec_prepared_hlpr(E &e, A&& a, B && ... b){
					auto fwd = e(std::forward<A>(a));
					return exec_prepared_hlpr(fwd,std::forward<B>(b)...);
				}
				
				template<typename Arg1, typename... Args>
				auto prepared(LocalTransactionNames name, const std::string &stmt,
											  Arg1 && a1, Args && ... args){
					auto nameint = (int) name;
					auto namestr = std::to_string(nameint);
					if (!sql_conn->prepared.at(nameint)){
						sql_conn->conn.prepare(namestr,stmt);
						sql_conn->prepared[nameint] = true;
					}
					auto fwd = trans.prepared(namestr)(std::forward<Arg1>(a1));
					
					return exec_prepared_hlpr(fwd,std::forward<Args>(args)...);
				}

				void exec(const std::string &str){
					sendBack(trans.exec(str));
				}

				void store_commit() {
					trans.commit();
					aborted_or_committed = true;
				}

				//Commands:
				mutils::DeserializationManager dsm;
				virtual void select_version(char const * const bytes) = 0;
				virtual void select_version_data(char const * const bytes) = 0;
				virtual void update_data(char const * const bytes) = 0;
				virtual void initialize_with_id(char const * const bytes) = 0;
				virtual void increment(char const * const bytes) = 0;
				
				auto obj_exists(Name id){
					const static std::string query =
						"select ID from \"BlobStore\" where id = $1 union select id from \"IntStore\" where id = $1 limit 1";
					auto r = prepared(LocalTransactionNames::exists,query,id);
					return r;
				}
				
				void remove(Name id){
					const static std::string bs =
						"delete from \"BlobStore\" where ID = $1";
					const static std::string is =
						"delete from \"IntStore\" where ID = $1";
					prepared(LocalTransactionNames::Del1,bs,id);
					prepared(LocalTransactionNames::Del2,is,id);
				}
				
				
				void receiveSQLCommand(TransactionNames name, char const * const bytes){
					using namespace cmds;
					using namespace mutils;
					switch(name){
					case TransactionNames::exists:
						sendBack(obj_exists(*from_bytes<Name>(dsm,bytes)).size() > 0);
						break;
					case TransactionNames::Del:
						remove(*from_bytes<Name>(dsm,bytes));
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
					}
				}
			};
			
			
			template<>
			class LocalSQLTransaction<Level::strong> : public LocalSQLTransaction_super{
				
				static constexpr Level l = Level::strong;
				
				auto select_version_s(Table t, Name id){
					static const std::string bs =
						"select Version from \"BlobStore\" where ID=$1";
					static const std::string is =
						"select Version from \"IntStore\" where ID=$1";
					auto &s = (t == Table::BlobStore ? bs : is );
					return prepared((t == Table::BlobStore ?
									 LocalTransactionNames::select_version_s_b :
									 LocalTransactionNames::select_version_s_i
										),s,id);
				}
				
				auto select_version_data_s(Table t, Name id){
					assert(l == Level::strong);
					//std::cerr << "in select_data" << std::endl;
					//AtScopeEnd ase{[](){//std::cerr << "out" << std::endl;}};
					//discard(ase);
					static const std::string bs =
						"select version, data  from \"BlobStore\" where index = 0 and ID = $1";
					static const std::string is = "select version, data from \"IntStore\" where ID = $1 and index = 0";
					switch(t) {
					case Table::BlobStore : return prepared(LocalTransactionNames::select1,bs,id); 
					case Table::IntStore : return prepared(LocalTransactionNames::select2,is,id); 
					}
					assert(false && "forgot a case");
				}				

				auto update_data_s(Table t, Name id, char const * const b){
					static const std::string bs =
						"update \"BlobStore\" set data=$2,Version=Version + 1 where ID=$1 returning version";
					static const std::string is =
						"update \"IntStore\" set data=$2,Version=Version + 1 where ID=$1 returning version";
					switch(t) {
					case Table::BlobStore : return prepared(LocalTransactionNames::Updates1,bs,id,make_blob(*from_bytes<Bytes>(b)));
					case Table::IntStore : return prepared(LocalTransactionNames::Updates2,is,id,((int*)b)[0]);
					}
					assert(false && "forgot a case");
				}
				
				auto increment_s(Table t, Name id){
					assert(t == Table::IntStore
						   && "Error: increment currently only defined on integers");
					const static std::string s =
						"update \"IntStore\" set data = data + 1 where id = $1 returning version";
					return prepared(LocalTransactionNames::Increment,s,id);
				}				

				void initialize_with_id_s(Table t, Name id, char const * const b){
					const static std::string bs =
						"INSERT INTO \"BlobStore\" (id,data) VALUES ($1,$2)";
					const static std::string is =
						"INSERT INTO \"IntStore\" (id,data) VALUES ($1,$2)";
					switch(t) {
					case Table::BlobStore : prepared(LocalTransactionNames::Insert1,bs,id,make_blob(*from_bytes<Bytes>(b))); return;
					case Table::IntStore : prepared(LocalTransactionNames::Insert2,is,id,b,((int*)b)[0]); return;
					}
					assert(false && "forgot a case");
				}

				
				extract_version(const pqxx::results &r){
					int where;
					assert(!res.empty());
					bool worked = res[0][0].to(where);
					assert(worked);
					assert(where != -1);
					return where;
				}
				
				void select_version(char const * const bytes){
					auto table = mutils::from_bytes<Table>(this->dsm,bytes);
					auto id = mutils::from_bytes<Name>(this->dsm,bytes + mutils::bytes_size(*table));
					sendBack(extract_version(select_version_s(*table,*id)));
				}
				
				void select_version_data(char const * const bytes){
					auto table = mutils::from_bytes<Table>(this->dsm,bytes);
					auto id = mutils::from_bytes<Name>(this->dsm,bytes + mutils::bytes_size(*table));
					auto r = select_version_data_s(*table,*id);
					sendBack(extract_version(r));
					if (*table == Table::BlobStore){
						binarystring bs{r[0][1]};
						sendBack(mutils::Bytes{bs.data(),bs.size()});
					}
					else {
						int send{-1};
						r[0][1].to(send);
						assert(send >= 0);
						sendBack(send);
					}
				}
				
				void update_data(char const * const bytes){
					std::unique_ptr<Table> t; std::unique_ptr<int> k;
					std::unique_ptr<Name> id; std::unique_ptr<std::array<int,NUM_CAUSAL_GROUPS> > ends; 
					auto offset = from_bytes(this->dsm,l,t,k,id,ends);
					sendBack(extract_version(update_data_s(*t,*id,bytes + offset)));
				}
				
				void initialize_with_id(char const * const bytes){
					std::unique_ptr<Table> t; std::unique_ptr<Name> id;
					auto offset = from_bytes(this->sm,t,id);
					initialize_with_id_s(*t,*id,offset + bytes);
				}
				
				void increment(char const * const bytes){
					sendBack(increment_s(Table::IntStore,*from_bytes<Name>(bytes)));
				}
			};
			
			template<>
			class LocalSQLTransaction<Level::causal> : public LocalSQLTransaction_super{
				static constexpr Level l = Level::strong;
				
				constexpr int group_mapper(int k){
					if (k < 1) assert(false && "Error: k is 1-indexed");
					constexpr int sizes = NUM_CAUSAL_GROUPS / NUM_CAUSAL_MASTERS;
					constexpr int overflow = NUM_CAUSAL_GROUPS - sizes * NUM_CAUSAL_MASTERS;
					return (k < overflow ? 0
							: (k - overflow - 1) / (NUM_CAUSAL_MASTERS)) + 1;
				}
				
				constexpr int md(int k){
					return (k % NUM_CAUSAL_GROUPS == 0 ? NUM_CAUSAL_GROUPS : k % NUM_CAUSAL_GROUPS);
				}
				
				constexpr auto update_data_c_cmd(LocalTransactionNames const (&xx)[Table_max * NUM_CAUSAL_GROUPS],
												 char const * const set) {
					using namespace std;
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
				
				auto select_version_c(Table t, Name id){
					using namespace std;
					static const vector<pair<LocalTransactionNames,string> > v = [&](){
						vector<pair<LocalTransactionNames,string> > v;
						for (int i = 0; i < Table_max; ++i){
							v.push_back(pair<LocalTransactionNames,string>{
									((Table)i == Table::IntStore ?
									 LocalTransactionNames::Sel1i : LocalTransactionNames::Sel1b),
										string("select vc1,vc2,vc3,vc4 from ") + table_name((Table)i).str
										+ " where ID=$1 and index=0"});
						}
						return v;
					}();
					auto &s = v.at((int)t);
					return prepared(s.first,s.second,id);
				}
				
				auto select_version_data_c(Table t, Name id){
					assert(l == Level::causal);
					//std::cerr << "in select_data" << std::endl;
					//AtScopeEnd ase{[](){//std::cerr << "out" << std::endl;}};
					//discard(ase);
					static const std::string bs =
						"select vc1,vc2,vc3,vc4, data from \"BlobStore\" where index = 0 and ID = $1";
					static const std::string is = "select vc1,vc2,vc3,vc4, data from \"IntStore\" where index = 0 and ID = $1";
					switch(t) {
					case Table::BlobStore : return prepared(LocalTransactionNames::select1,bs,id); 
					case Table::IntStore : return prepared(LocalTransactionNames::select2,is,id); 
					}
				assert(false && "forgot a case");
				}				

				auto update_data_c(Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, char const* const b){
					static const constexpr LocalTransactionNames tnames[NUM_CAUSAL_GROUPS * Table_max] = {
						LocalTransactionNames::udc1,LocalTransactionNames::udc4,LocalTransactionNames::udc7,
						LocalTransactionNames::udc2,LocalTransactionNames::udc5,LocalTransactionNames::udc8,
						LocalTransactionNames::udc3,LocalTransactionNames::udc6,
					};
					static const constexpr auto update_cmds = update_data_c_cmd(tnames,"$5");
					auto p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
					return prepared(p.first,p.second,id,ends[md(k+1)-1],ends[md(k+2)-1],ends[md(k+3)-1],(t == Table::IntStore ? *((int*)b) : make_blob(*from_bytes<Bytes>(b)));
				}
				
				
				auto increment_c(Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends){
					static const constexpr LocalTransactionNames tnames[NUM_CAUSAL_GROUPS * Table_max] = {
						LocalTransactionNames::ic1,LocalTransactionNames::ic4,LocalTransactionNames::ic7,
						LocalTransactionNames::ic2,LocalTransactionNames::ic5,LocalTransactionNames::ic8,
						LocalTransactionNames::ic3,LocalTransactionNames::ic6,
					};
					static const constexpr auto update_cmds = update_data_c_cmd(tnames,"data + 1");
					auto p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
					return prepared(p.first,p.second,id,ends[md(k+1)-1],ends[md(k+2)-1],ends[md(k+3)-1]);
				}
				
				template<typename Blob>
				void initialize_with_id_c(Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, const Blob& b){
					assert(k > 0);
					
					const static std::string bs = "INSERT INTO \"BlobStore\" (index,id) VALUES (0,$1), (1,$1), (2,$1)";
					const static std::string is = "INSERT INTO \"IntStore\" (index,id) VALUES (0,$1), (1,$1), (2,$1)";
					if (t == Table::IntStore){
						prepared(LocalTransactionNames::initci,is,id);
					} else prepared(LocalTransactionNames::initcb,bs,id);
					
					static const constexpr LocalTransactionNames tnames[NUM_CAUSAL_GROUPS * Table_max] = {
						LocalTransactionNames::udc1,LocalTransactionNames::udc4,LocalTransactionNames::udc7,
						LocalTransactionNames::udc2,LocalTransactionNames::udc5,LocalTransactionNames::udc8,
						LocalTransactionNames::udc3,LocalTransactionNames::udc6
					};
					constexpr const auto update_cmds = update_data_c_cmd(tnames,"$5");
					const auto &p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
					prepared(p.first,p.second,id,ends[md(k+1) - 1],ends[md(k+2) - 1],ends[md(k+3) - 1],b);
				}

				auto extract_version(const pqxx::results &r){
					std::array<int,NUM_CAUSAL_GROUPS> vers;
					assert(!r.empty());
					auto res1 = r[0][0].to(vers[0]);
					assert(res1);
					auto res2 = r[0][1].to(vers[1]);
					assert(res2);
					auto res3 = r[0][2].to(vers[2]);
					assert(res3);
					auto res4 = r[0][3].to(vers[3]);
					assert(res4);
					return vers;
				}
				
				void select_version(char const * const bytes){
					auto table = mutils::from_bytes<Table>(this->dsm,bytes);
					auto id = mutils::from_bytes<Name>(this->dsm,bytes + mutils::bytes_size(*table));
					sendBack(extract_version(select_version_c(*table,*id)));
				}
				
				void select_version_data(char const * const bytes){
					auto table = mutils::from_bytes<Table>(this->dsm,bytes);
					auto id = mutils::from_bytes<Name>(this->dsm,bytes + mutils::bytes_size(*table));
					auto r = select_version_data_c(*table,*id);
					sendBack(extract_version(r));
					if (*table == Table::BlobStore){
						binarystring bs{r[0][4]};
						sendBack(mutils::Bytes{bs.data(),bs.size()});
					}
					else {
						int send{-1};
						r[0][4].to(send);
						assert(send >= 0);
						sendBack(send);
					}
				}
				void update_data(char const * const bytes){
					std::unique_ptr<Table> t; std::unique_ptr<int> k;
					std::unique_ptr<Name> id; std::unique_ptr<std::array<int,NUM_CAUSAL_GROUPS> > ends; 
					from_bytes(this->dsm,l,t,k,id,ends);
					sendBack(extract_version(update_data_c(*t,*k,*id,*ends)));
				}
				void initialize_with_id(char const * const bytes){
					std::unique_ptr<Table> t; std::unique_ptr<int> k; std::unique_ptr<Name> id;
					std::unique_ptr<std::array<int,NUM_CAUSAL_GROUPS> > ends; 
					auto offset = from_bytes(this->dsm,t,k,id,ends,b);
					initialize_with_id_c(*t,*k,*id,*ends,bytes + offset);
				}
				void increment(char const * const bytes){
					std::unique_ptr<Name> id; std::unique_ptr<std::array<int,NUM_CAUSAL_GROUPS> > ends;
					from_bytes(this->dsm,id,ends);
					sendBack(increment(*id,*ends));
				}
			};
		}
	}
}
