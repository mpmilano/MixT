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

		SQLStore_impl::SQLStore_impl(GDataStore &store, /*int instanceID,*/ Level l)
			:_store(store),clock{{0,0,0,0}},level(l),default_connection{new SQLConnection()} {
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


		int SQLStore_impl::ds_id() const{
			return 2 + (int) level;
		}

		int SQLStore_impl::instance_id() const{
			return default_connection->ip_addr;
		}

		SQLStore_impl::SQLConnection::SQLConnection()
			:prepared(((std::size_t) TransactionNames::MAX),false),conn{std::string("host=") + string_of_ip(ip_addr)}{
			static_assert(int{CAUSAL_GROUP} > 0, "errorr: did not set CAUSAL_GROUP or failed to 1-index");
			assert(conn.is_open());
			//std::cout << string_of_ip(ip) << std::endl;
		}
		const int SQLStore_impl::SQLConnection::repl_group;
		const unsigned int SQLStore_impl::SQLConnection::ip_addr;



		
		SQLStore_impl& SQLInstanceManager_abs::inst(Level l){
			if (l == Level::strong) return this->inst_strong();
			else if (l == Level::causal) return this->inst_causal();
			else assert(false && "what?");
		}




		SQLStore_impl::~SQLStore_impl(){
			delete default_connection;
                }//*/

	}
}
