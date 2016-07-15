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
#include "SQL_internal_utils.hpp"

namespace myria{ namespace pgsql {
		
		using namespace pqxx;
		using namespace std;
		using namespace mtl;
		using namespace tracker;
		using namespace mutils;
		using Internals = SQLStore_impl::GSQLObject::Internals;

		SQLStore_impl::SQLStore_impl(GDataStore &store, /*int instanceID,*/ Level l)
			:_store(store),clock{{0,0,0,0}},level(l) {
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
				new SQLTransaction(_store,default_connection.lock(),why));
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
		
		SQLStore_impl& SQLInstanceManager_abs::inst(Level l){
			if (l == Level::strong) return this->inst_strong();
			else if (l == Level::causal) return this->inst_causal();
			else assert(false && "what?");
		}


		SQLStore_impl::~SQLStore_impl(){}

	}
}

/*
GSQLObject
SQLConnection
SQLTransaction
*/
