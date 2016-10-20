//oh look, a source file! We remember those.
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
		
		SQLTransaction::SQLTransaction(GDataStore& store, LockedSQLConnection c, std::string why)
			:gstore(store),sql_conn(std::move(c)),
			 why(why)
		{
			assert(!sql_conn->in_trans());
			sql_conn->current_trans = this;
			char start_trans{4};
			sql_conn->conn.send(start_trans);
		}
		
		std::list<SQLStore_impl::GSQLObject*> objs;
		void SQLTransaction::add_obj(SQLStore_impl::GSQLObject* gso){
			objs.push_back(gso);
		}

		void SQLTransaction::store_abort(){
			if(!remote_aborted){
				char trans{1};
				sql_conn->conn.send(trans);
			}
			commit_on_delete = false;
		}
		
		SQLTransaction::~SQLTransaction(){
			auto &sql_conn = this->sql_conn;
			AtScopeEnd ase{[&sql_conn](){
					sql_conn->current_trans = nullptr;
				}};
			if (commit_on_delete) {
				store_commit();
			}
			else {
				store_abort();
			}
		}
	}
}
