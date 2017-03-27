//oh look, a source file! We remember those.
#include <arpa/inet.h>
#include "SQLStore_impl.hpp"
#include "SQLTransaction.hpp"
#include "Tracker_common.hpp"
#include "SQLStore.hpp"
#include "Ends.hpp"
#include "Ostreams.hpp"
#include "SafeSet.hpp"

namespace myria{ namespace pgsql {
		
		using namespace std;
		using namespace mtl;
		using namespace tracker;
		using namespace mutils;
		
		SQLTransaction::SQLTransaction(Level level, GDataStore& store, LockedSQLConnection c whendebug(, std::string why))
			:gstore(store),level(level),level_string(gstore.level_description),sql_conn(std::move(c))
			 whendebug(,why(why))
		{
			assert(!sql_conn->in_trans());
			sql_conn->current_trans = this;
			char start_trans{4};
			log_send(level_string + " start");
			sql_conn->conn->send(start_trans);
		}
		
		std::list<SQLStore_impl::GSQLObject*> objs;
		void SQLTransaction::add_obj(SQLStore_impl::GSQLObject* gso){
			objs.push_back(gso);
		}

		void SQLTransaction::store_abort(){
			assert(!committed_or_aborted);
			if(!remote_aborted){
				char trans{1};
				log_send(level_string + " abort");
				sql_conn->conn->send(trans);
			}
			commit_on_delete = false;
			committed_or_aborted = true;
		}
		
		SQLTransaction::~SQLTransaction(){
			auto &sql_conn = this->sql_conn;
			AtScopeEnd ase{[&sql_conn](){
					sql_conn->current_trans = nullptr;
				}};
			if (!committed_or_aborted){
				if (commit_on_delete) {
					store_commit();
				}
				else {
					store_abort();
				}
			}
		}

#ifndef NDEBUG
		void SQLTransaction::log_receive_start(const std::string& event_id){
			log_file << "starting " << event_id << std::endl;
			log_file.flush();
		}
		void SQLTransaction::log_receive_stop(const std::string& event_id){
			log_file << "done " << event_id << std::endl;
			log_file.flush();
		}
		
		void SQLTransaction::log_send(const std::string& event_id){
			log_file << "sending " << event_id << std::endl;
			log_file.flush();
		}
#endif
	}
}
