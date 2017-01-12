#pragma once
#include "LocalSQLTransaction.hpp"
#include "LocalSQLConnection.hpp"
#include "Bytes.hpp"

namespace myria { namespace pgsql {

		namespace local{

			using namespace mutils;

			template<typename Trans>
			void obj_exists(Trans &trans, Name id, mutils::connection& socket){
				const static std::string query =
					"select * from (select 1::boolean as one from \"BlobStore\" where id = $1 union select 1::boolean from \"IntStore\" where id = $1 limit 1) as sq1 union select 0::boolean as one where not exists (select 1::boolean as one from \"BlobStore\" where id = $1 union select  1::boolean from \"IntStore\" where id = $1 limit 1)";
				trans.template prepared<std::function<void (bool)> >([&](bool b){
						trans.sendBack("obj_exists", b,socket);
					},*trans.conn, LocalTransactionNames::exists,query,id);
			}

			template<typename Trans>
			void remove(Trans& trans, Name id, mutils::connection& ){
				const static std::string bs =
					"delete from \"BlobStore\" where ID = $1";
				const static std::string is =
					"delete from \"IntStore\" where ID = $1";
				trans.prepared(noop,*trans.conn,LocalTransactionNames::Del1,bs,id);
				trans.prepared(noop,*trans.conn,LocalTransactionNames::Del2,is,id);
			}

			

			LocalSQLTransaction_super::LocalSQLTransaction_super(LocalSQLConnection_super &conn whendebug(, std::ofstream& log_file)):
				trans(conn,mutils::gensym()) whendebug(,log_file(log_file))
			{
				log_receive(log_file,"transaction start");
			}
			
		
			template<typename T>
			void LocalSQLTransaction_super::sendBack(const std::string & whendebug(message), const T& t,mutils::connection& socket){
				log_send(log_file,message);
				socket.send(t);
			}
			
			template<typename F, typename Arg1, typename... Args>
			void LocalSQLTransaction_super::prepared(const F& f,LocalSQLConnection_super& sql_conn, LocalTransactionNames name, const std::string &stmt,
													 Arg1 && a1, Args && ... args){
				auto nameint = (int) name;
				auto namestr = std::to_string(nameint);
				if (!sql_conn.prepared.at(nameint)){
					sql_conn.prepare<Arg1,Args...>(namestr,stmt);
					sql_conn.prepared[nameint] = true;
				}
				

#ifndef NDEBUG
				log_file << "beginning actual SQL command" << std::endl;
				log_file.flush();
				mutils::AtScopeEnd ase{[&](){
						log_file << "SQL command complete" << std::endl;
						log_file.flush();
					}};
#endif
				trans.exec_prepared(f,namestr,std::forward<Arg1>(a1),std::forward<Args>(args)...);
			}
			
			//STRONG SECTION

			void LocalSQLTransaction<Level::strong>::select_version_s(std::function<void (long int)> action, Table t, Name id){
					static const std::string bs =
						"select Version from \"BlobStore\" where ID=$1";
					static const std::string is =
						"select Version from \"IntStore\" where ID=$1";
					auto &s = (t == Table::BlobStore ? bs : is );
					prepared(action,*conn,(t == Table::BlobStore ?
									 LocalTransactionNames::select_version_s_b :
									 LocalTransactionNames::select_version_s_i
										),s,id);
				}
				
			void LocalSQLTransaction<Level::strong>::select_version_data_s(std::function<void (long int, long int)> action, Table whendebug(t), Name id){
				assert(l == Level::strong);
				assert(t == Table::IntStore);
				static const std::string is = "select version, data from \"IntStore\" where ID = $1 and index = 0";
				prepared(action,*conn,LocalTransactionNames::select2,is,id);
			}

			void LocalSQLTransaction<Level::strong>::select_version_data_s(std::function<void (long int, mutils::Bytes)> action, Table whendebug(t) , Name id){
				assert(l == Level::strong);
				static const std::string bs =
					"select version, data  from \"BlobStore\" where index = 0 and ID = $1";
				assert(t == Table::BlobStore);
				prepared(action,*conn,LocalTransactionNames::select1,bs,id);
			}

			void LocalSQLTransaction<Level::strong>::update_data_s(std::function<void (long int)> action, Table t, Name id, char const * const b){
				static const std::string bs =
					"update \"BlobStore\" set data=$2,Version=Version + 1 where ID=$1 returning version";
				static const std::string is =
					"update \"IntStore\" set data=$2,Version=Version + 1 where ID=$1 returning version";
				switch(t) {
				case Table::BlobStore : return prepared(action,*conn,LocalTransactionNames::Updates1,bs,id,(*mutils::from_bytes_noalloc<mutils::Bytes>(&this->dsm,b)));
				case Table::IntStore : return prepared(action,*conn,LocalTransactionNames::Updates2,is,id,((int*)b)[0]);
				}
				assert(false && "forgot a case");
					struct dead_code{}; throw dead_code{};
			}
				
			void LocalSQLTransaction<Level::strong>::increment_s(std::function<void (long int)> action, Table t, Name id){
				(void)t;
				assert(t == Table::IntStore
					   && "Error: increment currently only defined on integers");
				const static std::string s =
					"update \"IntStore\" set data = data + 1 where id = $1 returning version";
				return prepared(action,*conn,LocalTransactionNames::Increment,s,id);
			}				

			void LocalSQLTransaction<Level::strong>::initialize_with_id_s(Table t, Name id, char const * const b){
					const static std::string bs =
						"INSERT INTO \"BlobStore\" (id,data) VALUES ($1,$2)";
					const static std::string is =
						"INSERT INTO \"IntStore\" (id,data) VALUES ($1,$2)";
					switch(t) {
					case Table::BlobStore : prepared(noop,*conn,LocalTransactionNames::Insert1,bs,id,(*mutils::from_bytes_noalloc<mutils::Bytes>(&this->dsm,b))); return;
					case Table::IntStore : prepared(noop,*conn,LocalTransactionNames::Insert2,is,id,((int*)b)[0]); return;
					}
					assert(false && "forgot a case");
					struct dead_code{}; throw dead_code{};
				}

			void LocalSQLTransaction<Level::strong>::obj_exists(Name id, mutils::connection& socket){
				return ::myria::pgsql::local::obj_exists(*this,id,socket);
			}
			void LocalSQLTransaction<Level::strong>::remove(Name id, mutils::connection& socket){
				return ::myria::pgsql::local::remove(*this,id,socket);
			}
				
			void LocalSQLTransaction<Level::strong>::select_version(char const * const bytes, mutils::connection& socket){
				auto table = mutils::from_bytes_noalloc<Table>(&this->dsm,bytes);
				auto id = mutils::from_bytes_noalloc<Name>(&this->dsm,bytes + mutils::bytes_size(*table));
				select_version_s([&](long int r){sendBack("select_version", (int)r, socket);},
								 *table,*id);
			}
				
			void LocalSQLTransaction<Level::strong>::select_version_data(char const * const bytes, mutils::connection& socket){
				auto table = mutils::from_bytes_noalloc<Table>(&this->dsm,bytes);
				auto id = mutils::from_bytes_noalloc<Name>(&this->dsm,bytes + mutils::bytes_size(*table));
				
				if (*table == Table::BlobStore){
					select_version_data_s([&](long int version, mutils::Bytes data){
							sendBack("version",(int)version,socket);
							sendBack("data",data,socket);
						},*table,*id);
				}
				else {
					select_version_data_s([&](long int version, long int data){
							sendBack("version",(int)version,socket);
							sendBack("data",(int)data,socket);
						},*table,*id);
				}
			}
				
			void LocalSQLTransaction<Level::strong>::update_data(char const * const bytes, mutils::connection& socket){
					std::unique_ptr<Table> t; std::unique_ptr<int> k;
					std::unique_ptr<Name> id; std::unique_ptr<std::array<int,NUM_CAUSAL_GROUPS> > ends; 
					auto offset = mutils::from_bytes_noalloc_v(&this->dsm,bytes,t,k,id,ends);
					update_data_s([&](long int version){sendBack("version",(int)version,socket);},
								  *t,*id,bytes + offset);
				}
				
			void LocalSQLTransaction<Level::strong>::initialize_with_id(char const * const bytes, mutils::connection& ){
					std::unique_ptr<Table> t; std::unique_ptr<Name> id;
					auto offset = mutils::from_bytes_noalloc_v(&this->dsm,bytes,t,id);
					initialize_with_id_s(*t,*id,offset + bytes);
				}
				
			void LocalSQLTransaction<Level::strong>::increment(char const * const bytes, mutils::connection& socket){
				increment_s([&](long int vers){sendBack("version",(int)vers,socket);},
							Table::IntStore,*mutils::from_bytes_noalloc<Name>(&this->dsm,bytes));
			}

			std::unique_ptr<LocalSQLConnection<Level::strong> > LocalSQLTransaction<Level::strong>::store_abort(std::unique_ptr<LocalSQLTransaction<Level::strong> > o) {
				whendebug(o->log_receive(o->log_file,"aborting");)
				o->aborted_or_committed = true;
				o->trans.abort(noop);
				return std::move(o->conn);
			}

			//CAUSAL LAND

			void LocalSQLTransaction<Level::causal>::select_version_c(std::function<void (long int, long int, long int, long int)> action, Table t, Name id){
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
					return prepared(action,*conn,s.first,s.second,id);
				}
				
			void LocalSQLTransaction<Level::causal>::select_version_data_c(std::function<void (long int, long int, long int, long int, long int)> action, Table whendebug(t), Name id){
				assert(l == Level::causal);
				assert(t == Table::IntStore);
				static const std::string is = "select vc1,vc2,vc3,vc4, data from \"IntStore\" where index = 0 and ID = $1";
				prepared(action,*conn,LocalTransactionNames::select2,is,id);
			}

			void LocalSQLTransaction<Level::causal>::select_version_data_c(std::function<void (long int, long int, long int, long int, mutils::Bytes)> action, Table whendebug(t), Name id){
				assert(l == Level::causal);
				assert(t == Table::BlobStore);
				static const std::string bs =
					"select vc1,vc2,vc3,vc4, data from \"BlobStore\" where index = 0 and ID = $1";
				prepared(action, *conn,LocalTransactionNames::select1,bs,id);
			}

			void LocalSQLTransaction<Level::causal>::update_data_c(std::function<void (long int, long int, long int, long int)> action,
																   Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, char const* const b){
				static const constexpr LocalTransactionNames tnames[NUM_CAUSAL_GROUPS * Table_max] = {
					LocalTransactionNames::udc1,LocalTransactionNames::udc4,LocalTransactionNames::udc7,
					LocalTransactionNames::udc2,LocalTransactionNames::udc5,LocalTransactionNames::udc8,
					LocalTransactionNames::udc3,LocalTransactionNames::udc6,
				};
				static const constexpr auto update_cmds = update_data_c_cmd(tnames,"$5");
				auto p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
#define argh_344234(x...) prepared(action,*conn,p.first,p.second,id,ends[md(k+1)-1],ends[md(k+2)-1],ends[md(k+3)-1],x)
				if (t == Table::IntStore){
					return argh_344234(*((int*)b));
				}
				else {
					return argh_344234((*mutils::from_bytes_noalloc<mutils::Bytes>(&this->dsm,b)));
				}
			}
				
			
			void LocalSQLTransaction<Level::causal>::increment_c(std::function<void (long int, long int, long int, long int)> action,
																 Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends){
				static const constexpr LocalTransactionNames tnames[NUM_CAUSAL_GROUPS * Table_max] = {
					LocalTransactionNames::ic1,LocalTransactionNames::ic4,LocalTransactionNames::ic7,
					LocalTransactionNames::ic2,LocalTransactionNames::ic5,LocalTransactionNames::ic8,
					LocalTransactionNames::ic3,LocalTransactionNames::ic6,
				};
				static const constexpr auto update_cmds = update_data_c_cmd(tnames,"data + 1");
				auto p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
				(void)action; (void) id; (void) ends;
				//TODO - debugging
				action(ends[0],ends[1],ends[2],ends[3]);
				//return prepared(action,*conn,p.first,p.second,id,ends[md(k+1)-1],ends[md(k+2)-1],ends[md(k+3)-1]);
			}
				

			template<typename Blob>
			void LocalSQLTransaction<Level::causal>::initialize_with_id_c(Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, const Blob &b){
				assert(k > 0);
				
				const static std::string bs = "INSERT INTO \"BlobStore\" (index,id) VALUES (0,$1), (1,$1), (2,$1)";
				const static std::string is = "INSERT INTO \"IntStore\" (index,id) VALUES (0,$1), (1,$1), (2,$1)";
				if (t == Table::IntStore){
					prepared(noop,*conn,LocalTransactionNames::initci,is,id);
				} else prepared(noop,*conn,LocalTransactionNames::initcb,bs,id);
				
				static const constexpr LocalTransactionNames tnames[NUM_CAUSAL_GROUPS * Table_max] = {
					LocalTransactionNames::udc1,LocalTransactionNames::udc4,LocalTransactionNames::udc7,
					LocalTransactionNames::udc2,LocalTransactionNames::udc5,LocalTransactionNames::udc8,
					LocalTransactionNames::udc3,LocalTransactionNames::udc6
				};
				constexpr const auto update_cmds = update_data_c_cmd(tnames,"$5");
				const auto &p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
				prepared(noop,*conn,p.first,p.second,id,ends[md(k+1) - 1],ends[md(k+2) - 1],ends[md(k+3) - 1],b);
			}
			
			void LocalSQLTransaction<Level::causal>::obj_exists(Name id,mutils::connection& socket){
				return ::myria::pgsql::local::obj_exists(*this,id,socket);
			}
			
			void LocalSQLTransaction<Level::causal>::remove(Name id,mutils::connection& socket){
				return ::myria::pgsql::local::remove(*this,id,socket);
			}
				
			void LocalSQLTransaction<Level::causal>::select_version(char const * const bytes, mutils::connection& socket){
					auto table = mutils::from_bytes_noalloc<Table>(&this->dsm,bytes);
					auto id = mutils::from_bytes_noalloc<Name>(&this->dsm,bytes + mutils::bytes_size(*table));
					select_version_c(
						[&](long int vc1, long int vc2, long int vc3, long int vc4){
							sendBack("version",std::array<int,4>{{(int)vc1,(int)vc2,(int)vc3,(int)vc4}},socket);
						},
						*table,*id);
				}
				
			void LocalSQLTransaction<Level::causal>::select_version_data(char const * const bytes, mutils::connection& socket){
					auto table = mutils::from_bytes_noalloc<Table>(&this->dsm,bytes);
					auto id = mutils::from_bytes_noalloc<Name>(&this->dsm,bytes + mutils::bytes_size(*table));
					if (*table == Table::BlobStore){
						select_version_data_c(
							[&](long int vc1, long int vc2, long int vc3, long int vc4, mutils::Bytes b){
								sendBack("version",std::array<int,4>{{(int)vc1,(int)vc2,(int)vc3,(int)vc4}},socket);
								sendBack("data",b,socket);
							},
							*table,*id);
						
					}
					else {
						select_version_data_c(
							[&](long int vc1, long int vc2, long int vc3, long int vc4, long int data){
								sendBack("version",std::array<int,4>{{(int)vc1,(int)vc2,(int)vc3,(int)vc4}},socket);
								sendBack("data",(int)data,socket);
							},
							*table,*id);
					}
				}
			
			void LocalSQLTransaction<Level::causal>::update_data(char const * const bytes,mutils::connection& socket){
					std::unique_ptr<Table> t; std::unique_ptr<int> k;
					std::unique_ptr<Name> id; std::unique_ptr<std::array<int,NUM_CAUSAL_GROUPS> > ends; 
					auto size = mutils::from_bytes_noalloc_v(&this->dsm,bytes,t,k,id,ends);
					update_data_c(
						[&](long int vc1, long int vc2, long int vc3, long int vc4){
							sendBack("version",std::array<int,4>{{(int)vc1,(int)vc2,(int)vc3,(int)vc4}},socket);
						},
						*t,*k,*id,*ends,bytes + size);
				}
			void LocalSQLTransaction<Level::causal>::initialize_with_id(char const * const ,mutils::connection& ){
				//std::unique_ptr<Table> t; std::unique_ptr<int> k; std::unique_ptr<Name> id;
					//std::unique_ptr<std::array<int,NUM_CAUSAL_GROUPS> > ends; 
					//auto offset = mutils::from_bytes_v(&this->dsm,bytes,t,k,id,ends);
					struct unsupported_operation : public std::exception {
						const char* what() const noexcept { return "unsupported";}
					};
					throw unsupported_operation{};
					//initialize_with_id_c(*t,*k,*id,*ends,bytes + offset);
			}
			void LocalSQLTransaction<Level::causal>::increment(char const * const bytes,mutils::connection& socket){
				std::unique_ptr<int> k;
				std::unique_ptr<Name> id; std::unique_ptr<std::array<int,NUM_CAUSAL_GROUPS> > ends;
				mutils::from_bytes_noalloc_v(&this->dsm,bytes,k,id,ends);
				increment_c(
					[&](long int vc1, long int vc2, long int vc3, long int vc4){
						sendBack("version",std::array<int,4>{{(int)vc1,(int)vc2,(int)vc3,(int)vc4}},socket);
					},
					Table::IntStore,*k,*id,*ends);
			}

			std::unique_ptr<LocalSQLConnection<Level::causal> > LocalSQLTransaction<Level::causal>::store_abort(std::unique_ptr<LocalSQLTransaction<Level::causal> > o) {
				o->aborted_or_committed = true;
				whendebug(o->log_receive(o->log_file,"aborting"));
				o->trans.abort(noop);
				return std::move(o->conn);
			}
		}
	}
}
