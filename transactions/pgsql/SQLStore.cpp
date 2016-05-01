//oh look, a source file! We remember those.
#include <pqxx/pqxx>
#include <arpa/inet.h>
#include "SQLStore_impl.hpp"
#include "SQLTransaction.hpp"
#include "Tracker_common.hpp"
#include "SQLCommands.hpp"
#include "SQLStore.hpp"
#include "Ends.hpp"
#include "Ostreams.hpp"

namespace myria{ namespace pgsql {

		namespace{
			constexpr int max_ver_check_size = 100;
		}
		
		using namespace pqxx;
		using namespace std;
		using namespace mtl;
		using namespace tracker;
		using namespace mutils;
		using Internals = SQLStore_impl::GSQLObject::Internals;


		SQLTransaction::SQLTransaction(GDataStore& store, SQLStore_impl::SQLConnection& c, std::string why)
                        :gstore(store),sql_conn(c),conn_lock(
                            [](auto& l) -> auto& {
                                assert(l.try_lock());
                                l.unlock();
                                return l;
                        }(c.con_guard)),
						 trans(sql_conn.conn),
						 why(why)
		{
			assert(!sql_conn.in_trans());
			sql_conn.current_trans = this;
		}

		bool SQLTransaction::is_serialize_error(const pqxx::pqxx_exception &r){
			auto s = std::string(r.base().what());
			return s.find("could not serialize access") != std::string::npos;
		}
#define default_sqltransaction_catch									\
		catch(const pqxx::pqxx_exception &r){							\
			commit_on_delete = false;									\
			if (is_serialize_error(r)) throw mtl::SerializationFailure{}; \
			else throw mtl::CannotProceedError{r.base().what() /*+ mutils::show_backtrace()*/}; \
		}

	
		template<typename Arg1, typename... Args>
		auto SQLTransaction::prepared(const std::string &name, const std::string &stmt,
					  Arg1 && a1, Args && ... args){
			try{
				sql_conn.conn.prepare(name,stmt);
				auto fwd = trans.prepared(name)(std::forward<Arg1>(a1));
				return exec_prepared_hlpr(fwd,std::forward<Args>(args)...);
			}
				default_sqltransaction_catch
					}

		

		pqxx::result SQLTransaction::exec(const std::string &str){
				try{
					return trans.exec(str);
				}
				default_sqltransaction_catch
					}
		
	
		bool SQLTransaction::store_commit() {
			sql_conn.current_trans = nullptr;
			trans.commit();
			return true;
		}

		bool SQLStore_impl::SQLConnection::in_trans(){
                        if (current_trans){/*
				assert(con_guard.try_lock());
                                con_guard.unlock();*/
			}
			return current_trans;
		}

		void SQLTransaction::add_obj(SQLStore_impl::GSQLObject* gso){
			objs.push_back(gso);
		}

		void SQLTransaction::store_abort(){
			commit_on_delete = false;
		}
		
		SQLTransaction::~SQLTransaction(){
			auto &sql_conn = this->sql_conn;
			AtScopeEnd ase{[&sql_conn](){
					sql_conn.current_trans = nullptr;
				}};
			if (commit_on_delete) {
				store_commit();
			}
		}		

		SQLStore_impl::SQLStore_impl(GDataStore &store, int instanceID, Level l)
			:_store(store),clock{{0,0,0,0}},level(l),default_connection{new SQLConnection(instanceID)} {
				auto t = begin_transaction("Setting up this new SQLStore; gotta configure search paths and stuff.");
				((SQLTransaction*)t.get())
					->exec(l == Level::strong ?
						   "set search_path to \"BlobStore\",public"
						   : "set search_path to causalstore,public");
				((SQLTransaction*)t.get())
					->exec(l == Level::strong ?
						   "SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL SERIALIZABLE"
						   : "SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL REPEATABLE READ");
				auto cres = t->store_commit();
				assert(cres);
			}

		unique_ptr<SQLTransaction> SQLStore_impl::begin_transaction(const std::string &why)
		{
			assert(!default_connection->in_trans() &&
				   "Concurrency support doesn't exist yet."
				);
			return unique_ptr<SQLTransaction>(
				new SQLTransaction(_store,*default_connection,why));
		}

		namespace {
			unique_ptr<SQLTransaction> small_transaction(SQLStore_impl &store, const std::string &why) {
				unique_ptr<SQLTransaction> owner
					((SQLTransaction*)store.begin_transaction(why).release());
				owner->commit_on_delete = true;
				return owner;
			}
		}

		struct SQLStore_impl::GSQLObject::Internals{
			const Table table;
			const Name key;
			std::size_t size;
			const int store_id;
			const Level level;
			SQLStore_impl &_store;
			char* buf1;
			int vers;
			std::array<int,4> causal_vers;
			Internals(Table table, Name key, int size,
					  SQLStore_impl& store,char* buf)
				:table(table),key(key),size(size),store_id(store.instance_id()),level(store.level),_store(store),
				 buf1(buf),vers(-1),causal_vers{{-1,-1,-1,-1}}
				{}
		};

		SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl::GSQLObject&& gso)
			:i(gso.i){gso.i = nullptr;}

		namespace{

			auto enter_store_transaction(SQLStore_impl& store){
				unique_ptr<SQLTransaction> t_owner;
				SQLTransaction *trns = nullptr;
				if (!(store).default_connection->in_trans()){
					t_owner = small_transaction(store,"enter_store_transaction found no active transaction running");
					trns = t_owner.get();
				}
				else trns = (store).default_connection->current_trans;
				return make_pair(move(t_owner),trns);
			}
	
			pair<unique_ptr<SQLTransaction>, SQLTransaction*>
				enter_transaction(SQLStore_impl &store, SQLTransaction *trns){
				if (!trns){
					return enter_store_transaction(store);
				}
				else return make_pair(unique_ptr<SQLTransaction>{nullptr},trns);
			}

			//strong
			int process_version_update(const result &res, int& where){
				assert(!res.empty());
				bool worked = res[0][0].to(where);
				assert(worked);
				assert(where != -1);
				return 1;
			}
			
			//causal
			int process_version_update(const result &r, std::array<int,NUM_CAUSAL_GROUPS>& vers){
				assert(!r.empty());
				auto res1 = r[0][0].to(vers[0]);
				assert(res1);
				auto res2 = r[0][1].to(vers[1]);
				assert(res2);
				auto res3 = r[0][2].to(vers[2]);
				assert(res3);
				auto res4 = r[0][3].to(vers[3]);
				assert(res4);
				return 4;
			}
		}

		SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl &ss, Table t, Name id, int size)
			:i(new Internals{t,id,size,ss,nullptr}){
			i->buf1 = (char*) malloc(size);
			auto b = load(nullptr);
			assert(b);
		}

//existing object
		SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl& ss, Table t, Name id)
			:i{new Internals{t,id,
					(t == Table::IntStore ? (int) sizeof(int) : -1),
					ss,
					(t == Table::IntStore ? (char*) malloc(sizeof(int)) : nullptr)
					}} {}

//"named" object
		SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl& ss, Table t, Name id, const vector<char> &c)
			:i{new Internals{t,id,(int)c.size(),ss,
					(char*) malloc(c.size())}}{

			{
				auto trans_owner = enter_transaction(ss,nullptr);
				auto *trans = trans_owner.second;
                                assert(!ro_isValid(trans));

				if (t == Table::BlobStore){
					binarystring blob(&c.at(0),c.size());
					cmds::initialize_with_id(ss.level,*trans,t,ss.default_connection->repl_group,id,ss.clock,blob);
				}
				else if (t == Table::IntStore){
					cmds::initialize_with_id(ss.level,*trans,t,ss.default_connection->repl_group,id,ss.clock,((int*)c.data())[0]);
				}
                                if (i->_store.level == Level::causal){
                                    for (auto& val : i->causal_vers)
                                        val = 0;
                                }
                                else if (i->_store.level == Level::strong){
                                    i->vers = 0;
                                }
			}
			memcpy(this->i->buf1, &c.at(0), c.size());
		}

		SQLStore_impl& SQLStore_impl::GSQLObject::store() {
			return i->_store;
		}

		namespace {
			//transaction context needs to be different sometimes
			template<typename Trans>
			bool obj_exists(Name id, Trans owner){
				//level doesn't matter here for now.
				return cmds::obj_exists(Level::undef,*owner,id).size() > 0;
			}
		}

		bool SQLStore_impl::exists(Name id) {
			auto owner = enter_store_transaction(*this);
			return obj_exists(id,owner.second);
		}

		void SQLStore_impl::remove(Name id) {
			cmds::remove(level,*small_transaction(*this,std::string("Trying to remove something of name ") + std::to_string(id)),Table::BlobStore,id);
		}

		SQLStore_impl::GSQLObject::~GSQLObject(){
			if (i){
				if (i->buf1) free(i->buf1);
				delete i;
			}
		}

		int SQLStore_impl::ds_id() const{
			return 2 + (int) level;
		}

		int SQLStore_impl::instance_id() const{
			return default_connection->ip_addr;
		}

		SQLStore_impl::SQLConnection::SQLConnection(int ip):ip_addr(ip),repl_group(CAUSAL_GROUP),conn{std::string("host=") + string_of_ip(ip)}{
			static_assert(int{CAUSAL_GROUP} > 0, "errorr: did not set CAUSAL_GROUP or failed to 1-index");
			assert(conn.is_open());
			//std::cout << string_of_ip(ip) << std::endl;
		}

		int SQLStore_impl::GSQLObject::store_instance_id() const {
			return i->store_id;
		}

		Name SQLStore_impl::GSQLObject::name() const {
			return this->i->key;
		}

		const std::array<int,NUM_CAUSAL_GROUPS>& SQLStore_impl::GSQLObject::timestamp() const {
			return this->i->causal_vers;
		}

		void SQLStore_impl::GSQLObject::save(SQLTransaction *gso){
			auto owner = enter_transaction(store(),gso);
			auto trans = owner.second;
			char *c = i->buf1;
			assert(c);

#define upd_23425(x...) cmds::update_data(i->_store.level,*trans,i->table,i->_store.default_connection->repl_group,i->key,i->_store.clock,x)
	
			if (i->table == Table::BlobStore){
				binarystring blob(c,i->size);
                                auto res = upd_23425(blob);
                                if (i->_store.level == Level::strong){
                                    process_version_update(res,i->vers);
                                }
                                else if (i->_store.level == Level::causal){
                                    process_version_update(res,i->causal_vers);
                                }
			}
			else if (i->table == Table::IntStore){
                                auto res = upd_23425(((int*)c)[0]);
                                if (i->_store.level == Level::strong){
                                    process_version_update(res,i->vers);
                                }
                                else if (i->_store.level == Level::causal){
                                    process_version_update(res,i->causal_vers);
                                }
			}
		}

		char* SQLStore_impl::GSQLObject::load(SQLTransaction *gso){
			auto owner = enter_transaction(store(),gso);
			auto trans = owner.second;
			if (i->size >= max_ver_check_size){
				bool store_same = false;
				if (i->_store.level == Level::causal){
					auto old = i->causal_vers;
					process_version_update(cmds::select_version(i->_store.level, *trans,i->table,i->key),i->causal_vers);
					store_same = ends::is_same(old,i->causal_vers);
					if (!store_same) i->_store.clock = max(i->_store.clock,i->causal_vers);
					assert(i->_store.clock[2] < 30);
				}
				else if (i->_store.level == Level::strong){
					auto old = i->vers;
					int newi = -12;
                                        process_version_update(cmds::select_version(i->_store.level, *trans,i->table,i->key),newi);
					store_same = (old == newi);
					i->vers = newi;
				}
				if (store_same) return i->buf1;
			}
			{
                                result r = cmds::select_version_data_size(i->_store.level,*trans,i->table,i->key);
                                int start_offset = 0;
                                if (i->_store.level == Level::causal){
                                    start_offset = process_version_update(r,i->causal_vers);
                                }
                                else start_offset = process_version_update(r,i->vers);
				if (i->table == Table::BlobStore){
                                    bool wrkd = r[0][start_offset+1].to(i->size);
                                    assert(wrkd);
                                        binarystring bs(r[0][start_offset]);
					assert(bs.size() == i->size);
					assert(i->size >= 1);
					if (!i->buf1) i->buf1 = (char*) malloc(i->size);
					memcpy(i->buf1,bs.data(),i->size);
				}
				else if (i->table == Table::IntStore) {
					int res = -1;
                                        if (!r[0][start_offset].to(res)){
						std::cerr << "Attempting to access key "<< i->key << " from IntStore in " <<  i->_store.level << " land" << std::endl;
						assert(false && "no result!");
					}
					((int*)i->buf1)[0] = res;
				}
				assert(i->buf1);
				return i->buf1;
			}
		}

		void SQLStore_impl::GSQLObject::increment(SQLTransaction *gso){
			auto owner = enter_transaction(store(),gso);
                        auto r = cmds::increment(i->_store.level,
							*owner.second,
							i->table,
							i->_store.default_connection->repl_group,
							i->key,
							i->_store.clock);
                        if (i->_store.level == Level::causal){
                            process_version_update(r,i->causal_vers);
                        }
                        else process_version_update(r,i->vers);
		}

		bool SQLStore_impl::GSQLObject::ro_isValid(SQLTransaction *gso) const {
			auto& stre = const_cast<GSQLObject&>(*this).store();
			auto owner = enter_transaction(stre,gso);
			return obj_exists(i->key,owner.second);
		}

		void SQLStore_impl::GSQLObject::resize_buffer(int newsize){
			if(!i->buf1) {
				i->buf1 = (char*) malloc(newsize);
				i->size = newsize;
			}
			else assert(newsize == i->size);
		}

		char* SQLStore_impl::GSQLObject::obj_buffer() {
			assert(i->buf1);
			return i->buf1;
		}

		char const * SQLStore_impl::GSQLObject::obj_buffer() const {
			assert(i->buf1);
			return i->buf1;
		}

		int SQLStore_impl::GSQLObject::obj_buffer_size() const {
			assert(i->size >= 0);
			return i->size;
		}

		int SQLStore_impl::GSQLObject::bytes_size() const {
			return sizeof(int)*4 + sizeof(Level) + sizeof(Table);
		}

		int SQLStore_impl::GSQLObject::to_bytes(char* c) const {
			//TODO: this is not symmetric! That is a bad design! Bad!
			int* arr = (int*)c;
			arr[0] = (i->level == Level::strong ?
					  SQLStore<Level::strong>::id() :
					  SQLStore<Level::causal>::id());
			arr[1] = i->key;
			arr[2] = i->size;
			arr[3] = i->store_id;
			Level* arrl = (Level*) (arr + 4);
			arrl[0] = i->level;
			Table* arrt = (Table*) (arrl + 1);
			arrt[0] = i->table;
			return this->bytes_size();
		}
		
		SQLStore_impl& SQLInstanceManager_abs::inst(Level l, int store_id){
			if (l == Level::strong) return this->inst_strong(store_id);
			else if (l == Level::causal) return this->inst_causal(store_id);
			else assert(false && "what?");
		}

		SQLStore_impl::GSQLObject SQLStore_impl::GSQLObject::from_bytes(SQLInstanceManager_abs& mgr, char const *v){
			int* arr = (int*)v;
			//arr[0] has already been used to find this implementation
			Level* arrl = (Level*) (arr + 3);
			Table* arrt = (Table*) (arrl + 1);
			//of from_bytes
			Level lvl = arrl[0];
			return GSQLObject(mgr.inst(lvl,arr[2]),
							  arrt[0],arr[0],arr[1]);
		}


		SQLStore_impl::~SQLStore_impl(){
			delete default_connection;
                }//*/

	}
}
