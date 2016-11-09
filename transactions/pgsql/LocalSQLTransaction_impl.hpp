#pragma once
#include "LocalSQLTransaction.hpp"
#include "LocalSQLConnection.hpp"
#include "Bytes.hpp"
#include "BlobUtils.hpp"

namespace myria { namespace pgsql {

		namespace local{

			using namespace mutils;

			template<typename Trans>
			void obj_exists(Trans &trans, Name id, mutils::connection& socket){
				const static std::string query =
					"select ID from \"BlobStore\" where id = $1 union select id from \"IntStore\" where id = $1 limit 1";
				auto r = trans.prepared(*trans.conn, LocalTransactionNames::exists,query,id);
				trans.all_fine(socket);
				trans.sendBack("obj_exists", r.size() > 0,socket);
			}

			template<typename Trans>
			void remove(Trans& trans, Name id, mutils::connection& socket){
				const static std::string bs =
					"delete from \"BlobStore\" where ID = $1";
				const static std::string is =
					"delete from \"IntStore\" where ID = $1";
				trans.prepared(*trans.conn,LocalTransactionNames::Del1,bs,id);
				trans.prepared(*trans.conn,LocalTransactionNames::Del2,is,id);
				trans.all_fine(socket);
			}

			
			template<typename SQLConn>
			LocalSQLTransaction_super::LocalSQLTransaction_super(SQLConn &conn whendebug(, std::ofstream& log_file)):
				trans(conn.conn) whendebug(,log_file(log_file))
			{
				log_receive("transaction start");
			}
		
			template<typename T>
			void LocalSQLTransaction_super::sendBack(const std::string & whendebug(message), const T& t,mutils::connection& socket){
				log_send(message);
				socket.send(t);
			}
			
			template<typename E>
			auto LocalSQLTransaction_super::exec_prepared_hlpr(E &e){
				return e.exec();
			}
			
			template<typename E, typename A, typename... B>
			auto LocalSQLTransaction_super::exec_prepared_hlpr(E &e, A&& a, B && ... b){
				auto fwd = e(std::forward<A>(a));
				return exec_prepared_hlpr(fwd,std::forward<B>(b)...);
			}
			
			template<typename SQL_Conn, typename Arg1, typename... Args>
			auto LocalSQLTransaction_super::prepared(SQL_Conn& sql_conn, LocalTransactionNames name, const std::string &stmt,
													 Arg1 && a1, Args && ... args){
				auto nameint = (int) name;
				auto namestr = std::to_string(nameint);
				if (!sql_conn.prepared.at(nameint)){
					sql_conn.prepare(namestr,stmt);
					sql_conn.prepared[nameint] = true;
				}
				auto fwd = trans.prepared(namestr)(std::forward<Arg1>(a1));

#ifndef NDEBUG
				log_file << "beginning actual SQL command" << std::endl;
				log_file.flush();
				mutils::AtScopeEnd ase{[&](){
						log_file << "SQL command complete" << std::endl;
						log_file.flush();
					}};
#endif
				return exec_prepared_hlpr(fwd,std::forward<Args>(args)...);
			}
			
			//STRONG SECTION

			LocalSQLTransaction<Level::strong>::LocalSQLTransaction(std::unique_ptr<LocalSQLConnection<l> > conn whendebug(, std::ofstream& log_file))
				:LocalSQLTransaction_super(*conn whendebug (,log_file)),
				 conn(std::move(conn)){
				trans.exec("set search_path to \"BlobStore\",public");
				trans.exec("SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL SERIALIZABLE");
			}

			auto LocalSQLTransaction<Level::strong>::select_version_s(Table t, Name id){
					static const std::string bs =
						"select Version from \"BlobStore\" where ID=$1";
					static const std::string is =
						"select Version from \"IntStore\" where ID=$1";
					auto &s = (t == Table::BlobStore ? bs : is );
					return prepared(*conn,(t == Table::BlobStore ?
									 LocalTransactionNames::select_version_s_b :
									 LocalTransactionNames::select_version_s_i
										),s,id);
				}
				
				auto LocalSQLTransaction<Level::strong>::select_version_data_s(Table t, Name id){
					assert(l == Level::strong);
					//std::cerr << "in select_data" << std::endl;
					//AtScopeEnd ase{[](){//std::cerr << "out" << std::endl;}};
					//discard(ase);
					static const std::string bs =
						"select version, data  from \"BlobStore\" where index = 0 and ID = $1";
					static const std::string is = "select version, data from \"IntStore\" where ID = $1 and index = 0";
					switch(t) {
					case Table::BlobStore : return prepared(*conn,LocalTransactionNames::select1,bs,id); 
					case Table::IntStore : return prepared(*conn,LocalTransactionNames::select2,is,id); 
					}
					assert(false && "forgot a case");
					struct dead_code{}; throw dead_code{};
				}

				auto LocalSQLTransaction<Level::strong>::update_data_s(Table t, Name id, char const * const b){
					static const std::string bs =
						"update \"BlobStore\" set data=$2,Version=Version + 1 where ID=$1 returning version";
					static const std::string is =
						"update \"IntStore\" set data=$2,Version=Version + 1 where ID=$1 returning version";
					switch(t) {
					case Table::BlobStore : return prepared(*conn,LocalTransactionNames::Updates1,bs,id,make_blob(*from_bytes<Bytes>(&this->dsm,b)));
					case Table::IntStore : return prepared(*conn,LocalTransactionNames::Updates2,is,id,((int*)b)[0]);
					}
					assert(false && "forgot a case");
					struct dead_code{}; throw dead_code{};
				}
				
				auto LocalSQLTransaction<Level::strong>::increment_s(Table t, Name id){
					(void)t;
					assert(t == Table::IntStore
						   && "Error: increment currently only defined on integers");
					const static std::string s =
						"update \"IntStore\" set data = data + 1 where id = $1 returning version";
					return prepared(*conn,LocalTransactionNames::Increment,s,id);
				}				

			void LocalSQLTransaction<Level::strong>::initialize_with_id_s(Table t, Name id, char const * const b){
					const static std::string bs =
						"INSERT INTO \"BlobStore\" (id,data) VALUES ($1,$2)";
					const static std::string is =
						"INSERT INTO \"IntStore\" (id,data) VALUES ($1,$2)";
					switch(t) {
					case Table::BlobStore : prepared(*conn,LocalTransactionNames::Insert1,bs,id,make_blob(*from_bytes<Bytes>(&this->dsm,b))); return;
					case Table::IntStore : prepared(*conn,LocalTransactionNames::Insert2,is,id,b,((int*)b)[0]); return;
					}
					assert(false && "forgot a case");
					struct dead_code{}; throw dead_code{};
				}

			auto LocalSQLTransaction<Level::strong>::extract_version(const pqxx::result &res){
					int where;
					assert(!res.empty());
					bool worked = res[0][0].to(where);
					(void)worked;
					assert(worked);
					assert(where != -1);
					return where;
				}

			void LocalSQLTransaction<Level::strong>::obj_exists(Name id, mutils::connection& socket){
				return ::myria::pgsql::local::obj_exists(*this,id,socket);
			}
			void LocalSQLTransaction<Level::strong>::remove(Name id, mutils::connection& socket){
				return ::myria::pgsql::local::remove(*this,id,socket);
			}
				
			void LocalSQLTransaction<Level::strong>::select_version(char const * const bytes, mutils::connection& socket){
				auto table = mutils::from_bytes<Table>(&this->dsm,bytes);
				auto id = mutils::from_bytes<Name>(&this->dsm,bytes + mutils::bytes_size(*table));
				auto r = select_version_s(*table,*id);
				all_fine(socket);
				sendBack("select_version", extract_version(r), socket);
			}
				
			void LocalSQLTransaction<Level::strong>::select_version_data(char const * const bytes, mutils::connection& socket){
				auto table = mutils::from_bytes<Table>(&this->dsm,bytes);
				auto id = mutils::from_bytes<Name>(&this->dsm,bytes + mutils::bytes_size(*table));
				auto r = select_version_data_s(*table,*id);
				all_fine(socket);
				sendBack("version", extract_version(r),socket);
				if (*table == Table::BlobStore){
					pqxx::binarystring bs{r[0][1]};
					sendBack("data",mutils::Bytes{(char*)bs.data(),bs.size()},socket);
				}
				else {
					int send{-1};
					r[0][1].to(send);
					assert(send >= 0);
					sendBack("data",send,socket);
				}
			}
				
			void LocalSQLTransaction<Level::strong>::update_data(char const * const bytes, mutils::connection& socket){
					std::unique_ptr<Table> t; std::unique_ptr<int> k;
					std::unique_ptr<Name> id; std::unique_ptr<std::array<int,NUM_CAUSAL_GROUPS> > ends; 
					auto offset = from_bytes_v(&this->dsm,bytes,t,k,id,ends);
					auto r = update_data_s(*t,*id,bytes + offset);
					all_fine(socket);
					sendBack("version",extract_version(r),socket);
				}
				
			void LocalSQLTransaction<Level::strong>::initialize_with_id(char const * const bytes, mutils::connection& socket){
					std::unique_ptr<Table> t; std::unique_ptr<Name> id;
					auto offset = from_bytes_v(&this->dsm,bytes,t,id);
					initialize_with_id_s(*t,*id,offset + bytes);
					all_fine(socket);
				}
				
			void LocalSQLTransaction<Level::strong>::increment(char const * const bytes, mutils::connection& socket){
					auto r = increment_s(Table::IntStore,*from_bytes<Name>(&this->dsm,bytes));
					all_fine(socket);
					sendBack("version",extract_version(r),socket);
				}

			std::unique_ptr<LocalSQLConnection<Level::strong> > LocalSQLTransaction<Level::strong>::store_abort(std::unique_ptr<LocalSQLTransaction<Level::strong> > o,mutils::connection& ) {
				whendebug(o->log_receive("aborting");)
				o->aborted_or_committed = true;
				return std::move(o->conn);
			}

			void LocalSQLTransaction<Level::strong>::indicate_serialization_failure(mutils::connection& socket) {
				char abort{1};
				log_send("serialization failure");
				socket.send(abort);
			}

			void LocalSQLTransaction<Level::strong>::all_fine(mutils::connection& socket) {
				char ok{0};
				log_send("all fine");
				socket.send(ok);
			}



			//CAUSAL LAND

			LocalSQLTransaction<Level::causal>::LocalSQLTransaction(std::unique_ptr<LocalSQLConnection<l> > conn whendebug(, std::ofstream& log_file))
				:LocalSQLTransaction_super(*conn whendebug(, log_file)),
				 conn(std::move(conn)) {
				trans.exec("set search_path to causalstore,public");
				trans.exec("SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL REPEATABLE READ");
			}

			auto LocalSQLTransaction<Level::causal>::select_version_c(Table t, Name id){
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
					return prepared(*conn,s.first,s.second,id);
				}
				
				auto LocalSQLTransaction<Level::causal>::select_version_data_c(Table t, Name id){
					assert(l == Level::causal);
					//std::cerr << "in select_data" << std::endl;
					//AtScopeEnd ase{[](){//std::cerr << "out" << std::endl;}};
					//discard(ase);
					static const std::string bs =
						"select vc1,vc2,vc3,vc4, data from \"BlobStore\" where index = 0 and ID = $1";
					static const std::string is = "select vc1,vc2,vc3,vc4, data from \"IntStore\" where index = 0 and ID = $1";
					switch(t) {
					case Table::BlobStore : return prepared(*conn,LocalTransactionNames::select1,bs,id); 
					case Table::IntStore : return prepared(*conn,LocalTransactionNames::select2,is,id); 
					}
				assert(false && "forgot a case");
				struct dead_code{}; throw dead_code{};
				}				

				auto LocalSQLTransaction<Level::causal>::update_data_c(Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, char const* const b){
					static const constexpr LocalTransactionNames tnames[NUM_CAUSAL_GROUPS * Table_max] = {
						LocalTransactionNames::udc1,LocalTransactionNames::udc4,LocalTransactionNames::udc7,
						LocalTransactionNames::udc2,LocalTransactionNames::udc5,LocalTransactionNames::udc8,
						LocalTransactionNames::udc3,LocalTransactionNames::udc6,
					};
					static const constexpr auto update_cmds = update_data_c_cmd(tnames,"$5");
					auto p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
#define argh_344234(x...) prepared(*conn,p.first,p.second,id,ends[md(k+1)-1],ends[md(k+2)-1],ends[md(k+3)-1],x)
					if (t == Table::IntStore){
						return argh_344234(*((int*)b));
					}
					else {
						return argh_344234(make_blob(*from_bytes<Bytes>(&this->dsm,b)));
					}

				}
				
				
				auto LocalSQLTransaction<Level::causal>::increment_c(Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends){
					static const constexpr LocalTransactionNames tnames[NUM_CAUSAL_GROUPS * Table_max] = {
						LocalTransactionNames::ic1,LocalTransactionNames::ic4,LocalTransactionNames::ic7,
						LocalTransactionNames::ic2,LocalTransactionNames::ic5,LocalTransactionNames::ic8,
						LocalTransactionNames::ic3,LocalTransactionNames::ic6,
					};
					static const constexpr auto update_cmds = update_data_c_cmd(tnames,"data + 1");
					auto p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
					return prepared(*conn,p.first,p.second,id,ends[md(k+1)-1],ends[md(k+2)-1],ends[md(k+3)-1]);
				}
				
				template<typename Blob>
				void LocalSQLTransaction<Level::causal>::initialize_with_id_c(Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, const Blob& b){
					assert(k > 0);
					
					const static std::string bs = "INSERT INTO \"BlobStore\" (index,id) VALUES (0,$1), (1,$1), (2,$1)";
					const static std::string is = "INSERT INTO \"IntStore\" (index,id) VALUES (0,$1), (1,$1), (2,$1)";
					if (t == Table::IntStore){
						prepared(*conn,LocalTransactionNames::initci,is,id);
					} else prepared(*conn,LocalTransactionNames::initcb,bs,id);
					
					static const constexpr LocalTransactionNames tnames[NUM_CAUSAL_GROUPS * Table_max] = {
						LocalTransactionNames::udc1,LocalTransactionNames::udc4,LocalTransactionNames::udc7,
						LocalTransactionNames::udc2,LocalTransactionNames::udc5,LocalTransactionNames::udc8,
						LocalTransactionNames::udc3,LocalTransactionNames::udc6
					};
					constexpr const auto update_cmds = update_data_c_cmd(tnames,"$5");
					const auto &p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
					prepared(*conn,p.first,p.second,id,ends[md(k+1) - 1],ends[md(k+2) - 1],ends[md(k+3) - 1],b);
				}

				auto LocalSQLTransaction<Level::causal>::extract_version(const pqxx::result &r){
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
					(void)res1;
					(void)res2;
					(void)res3;
					(void)res4;
					return vers;
				}

			void LocalSQLTransaction<Level::causal>::obj_exists(Name id,mutils::connection& socket){
				return ::myria::pgsql::local::obj_exists(*this,id,socket);
				}
				
			void LocalSQLTransaction<Level::causal>::remove(Name id,mutils::connection& socket){
				return ::myria::pgsql::local::remove(*this,id,socket);
			}
				
			void LocalSQLTransaction<Level::causal>::select_version(char const * const bytes, mutils::connection& socket){
					auto table = mutils::from_bytes<Table>(&this->dsm,bytes);
					auto id = mutils::from_bytes<Name>(&this->dsm,bytes + mutils::bytes_size(*table));
					auto r = select_version_c(*table,*id);
					all_fine(socket);
					sendBack("version",extract_version(r),socket);
				}
				
			void LocalSQLTransaction<Level::causal>::select_version_data(char const * const bytes, mutils::connection& socket){
					auto table = mutils::from_bytes<Table>(&this->dsm,bytes);
					auto id = mutils::from_bytes<Name>(&this->dsm,bytes + mutils::bytes_size(*table));
					auto r = select_version_data_c(*table,*id);
					all_fine(socket);
					sendBack("version",extract_version(r),socket);
					if (*table == Table::BlobStore){
						pqxx::binarystring bs{r[0][4]};
						sendBack("data",mutils::Bytes{(char*)bs.data(),bs.size()},socket);
					}
					else {
						int send{-1};
						r[0][4].to(send);
						assert(send >= 0);
						sendBack("data",send,socket);
					}
				}
			void LocalSQLTransaction<Level::causal>::update_data(char const * const bytes,mutils::connection& socket){
					std::unique_ptr<Table> t; std::unique_ptr<int> k;
					std::unique_ptr<Name> id; std::unique_ptr<std::array<int,NUM_CAUSAL_GROUPS> > ends; 
					auto size = from_bytes_v(&this->dsm,bytes,t,k,id,ends);
					auto r = update_data_c(*t,*k,*id,*ends,bytes + size);
					all_fine(socket);
					sendBack("version",extract_version(r),socket);
				}
			void LocalSQLTransaction<Level::causal>::initialize_with_id(char const * const bytes,mutils::connection& socket){
					std::unique_ptr<Table> t; std::unique_ptr<int> k; std::unique_ptr<Name> id;
					std::unique_ptr<std::array<int,NUM_CAUSAL_GROUPS> > ends; 
					auto offset = from_bytes_v(&this->dsm,bytes,t,k,id,ends);
					initialize_with_id_c(*t,*k,*id,*ends,bytes + offset);
					all_fine(socket);
				}
			void LocalSQLTransaction<Level::causal>::increment(char const * const bytes,mutils::connection& socket){
					std::unique_ptr<int> k;
					std::unique_ptr<Name> id; std::unique_ptr<std::array<int,NUM_CAUSAL_GROUPS> > ends;
					from_bytes_v(&this->dsm,bytes,k,id,ends);
					auto r = increment_c(Table::IntStore,*k,*id,*ends);
					all_fine(socket);
					sendBack("version",extract_version(r),socket);
				}

			std::unique_ptr<LocalSQLConnection<Level::causal> > LocalSQLTransaction<Level::causal>::store_abort(std::unique_ptr<LocalSQLTransaction<Level::causal> > o,mutils::connection& ) {
				o->aborted_or_committed = true;
				whendebug(o->log_receive("aborting");)
				return std::move(o->conn);
				}
			
			void LocalSQLTransaction<Level::causal>::indicate_serialization_failure(mutils::connection& socket) {
				char abort{1};
				log_send("serialization failure");
				socket.send(abort);
			}

			void LocalSQLTransaction<Level::causal>::all_fine(mutils::connection& socket) {
				char ok{0};
				log_send("all fine");
				socket.send(ok);
			}

		}
	}
}
