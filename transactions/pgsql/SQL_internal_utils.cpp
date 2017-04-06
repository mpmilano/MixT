#include "SQL_internal_utils.hpp"

namespace myria{ namespace pgsql {

		using namespace pqxx;
		using namespace std;
		using namespace mtl;
		using namespace tracker;
		using namespace mutils;
		using Internals = SQLStore_impl::GSQLObject::Internals;

		unique_ptr<SQLTransaction> small_transaction(SQLStore_impl &store whendebug(, const std::string &why)) {
			unique_ptr<SQLTransaction> owner
				((SQLTransaction*)store.begin_transaction(whendebug(why)).release());
			owner->commit_on_delete = true;
			return owner;
		}
		
		std::pair<std::unique_ptr<SQLTransaction>, SQLTransaction*>
		enter_transaction(SQLStore_impl &, SQLTransaction *trns){
			assert(trns);
			return make_pair(unique_ptr<SQLTransaction>{nullptr},trns);
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
}
