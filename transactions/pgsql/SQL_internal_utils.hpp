#pragma once
#include <pqxx/pqxx>
#include <arpa/inet.h>
#include "pgsql/SQLStore_impl.hpp"
#include "pgsql/SQLTransaction.hpp"
#include "pgsql/SQLCommands.hpp"
#include "pgsql/SQLStore.hpp"
#include "tracker/Ends.hpp"
#include "../Ostreams.hpp"
#include "../mutils-containers/SafeSet.hpp"
namespace myria{ namespace pgsql {

		std::unique_ptr<SQLTransaction> small_transaction(SQLStore_impl &store whendebug(, const std::string &why));
		
		//strong
		int process_version_update(const pqxx::result &res, int& where);
		
		//causal
		int process_version_update(const pqxx::result &r, std::array<long long,NUM_CAUSAL_GROUPS>& vers);
		
		//transaction context needs to be different sometimes
		template<typename Trans>
		bool obj_exists(Name id, Trans owner){
			//level doesn't matter here for now.
			return cmds::obj_exists(Level::MAX,*owner,id).size() > 0;
		}
	}
}
