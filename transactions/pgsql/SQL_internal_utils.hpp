#pragma once
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

		std::unique_ptr<SQLTransaction> small_transaction(SQLStore_impl &store, const std::string &why);
		
		std::pair<std::unique_ptr<SQLTransaction>, SQLTransaction*> enter_store_transaction(SQLStore_impl& store);
		
		std::pair<std::unique_ptr<SQLTransaction>, SQLTransaction*>
		enter_transaction(SQLStore_impl &store, SQLTransaction *trns);

		//strong
		int process_version_update(const pqxx::result &res, int& where);
		
		//causal
		int process_version_update(const pqxx::result &r, std::array<int,NUM_CAUSAL_GROUPS>& vers);
		
		//transaction context needs to be different sometimes
		template<typename Trans>
		bool obj_exists(Name id, Trans owner){
			//level doesn't matter here for now.
			return cmds::obj_exists(Level::undef,*owner,id).size() > 0;
		}
	}
}
