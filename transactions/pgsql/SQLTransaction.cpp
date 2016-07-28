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
#include "SafeSet.hpp"

namespace myria{ namespace pgsql {
		
		using namespace pqxx;
		using namespace std;
		using namespace mtl;
		using namespace tracker;
		using namespace mutils;
		
		SQLTransaction::SQLTransaction(GDataStore& store, SQLStore_impl::LockedSQLConnection c, std::string why)
			:gstore(store),sql_conn(std::move(c)),conn_lock(
				[](auto& l) -> auto& {
					assert(l.try_lock());
					l.unlock();
					return l;
				}(sql_conn->con_guard)),
			 trans(sql_conn->conn),
			 why(why)
		{
			assert(!sql_conn->in_trans());
			assert(sql_conn->current_store);
			assert(!sql_conn->current_store->current_transaction);
			sql_conn->current_trans = this;
			sql_conn->current_store->current_transaction = this;
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
			sql_conn->current_trans = nullptr;
			trans.commit();
			return true;
		}

		void SQLTransaction::add_obj(SQLStore_impl::GSQLObject* gso){
			objs.push_back(gso);
		}

		void SQLTransaction::store_abort(){
			commit_on_delete = false;
		}
		
		SQLTransaction::~SQLTransaction(){
			
			AtScopeEnd ase{[this](){
					this->sql_conn->current_store->current_transaction = nullptr;
					this->sql_conn->current_trans = nullptr;
				}};
			if (commit_on_delete) {
				store_commit();
			}
		}
	}
}
