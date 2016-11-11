#pragma once
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

		std::unique_ptr<SQLTransaction> small_transaction(SQLStore_impl &store whendebug(, const std::string &why));
		
		std::pair<std::unique_ptr<SQLTransaction>, SQLTransaction*> enter_store_transaction(SQLStore_impl& store);
		
		std::pair<std::unique_ptr<SQLTransaction>, SQLTransaction*>
		enter_transaction(SQLStore_impl &store, SQLTransaction *trns);
		
		//transaction context needs to be different sometimes
		template<typename Trans>
		bool obj_exists(Name id, Trans owner){
			//level doesn't matter here for now.
			return owner->exists(id);
		}
	}
}
