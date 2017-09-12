//oh look, a source file! We remember those.
#include <arpa/inet.h>
#include "pgsql/SQLStore_impl.hpp"
#include "pgsql/SQLTransaction.hpp"
#include "pgsql/SQLCommands.hpp"
#include "pgsql/SQLStore.hpp"
#include "tracker/Ends.hpp"
#include "Ostreams.hpp"
#include "mutils-containers/SafeSet.hpp"

namespace myria{ namespace pgsql {
		
		using namespace pqxx;
		using namespace std;
		using namespace mtl;
		using namespace tracker;
		using namespace mutils;
		
		SQLTransaction::SQLTransaction(SQLStore_impl &whennopool(parent), GDataStore& store, LockedSQLConnection c whendebug(, std::string why))
			:gstore(store),whennopool(parent(parent),) sql_conn(std::move(c)),conn_lock(
				[](auto& l) -> auto& {
					assert(l.try_lock());
					l.unlock();
					return l;
				}(sql_conn->con_guard)),
			trans(sql_conn->conn)
			whendebug(,why(why))
		{
			assert(!sql_conn->in_trans());
			sql_conn->current_trans = this;
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
			auto &sql_conn = *this->sql_conn;
			AtScopeEnd ase{[&sql_conn](){
					sql_conn.current_trans = nullptr;
				}};
			if (commit_on_delete) {
				store_commit();
			}
			whennopool(parent.default_connection = std::move(this->sql_conn));
		}
	}
}
