#include "SQL_internal_utils.hpp"

namespace myria{ namespace pgsql {

		using namespace pqxx;
		using namespace std;
		using namespace mtl;
		using namespace tracker;
		using namespace mutils;
		using Internals = SQLStore_impl::GSQLObject::Internals;

	std::unique_ptr<SQLTransaction> small_transaction(SQLStore_impl &store, const std::string &why) {
		std::unique_ptr<SQLTransaction> owner
			((SQLTransaction*)store.begin_transaction(why).release());
		owner->commit_on_delete = true;
		return owner;
	}

	std::pair<std::unique_ptr<SQLTransaction>, SQLTransaction*> enter_store_transaction(SQLStore_impl& store){
		std::unique_ptr<SQLTransaction> t_owner;
		SQLTransaction *trns = nullptr;
		if (!(store).default_connection->in_trans()){
			t_owner = small_transaction(store,"enter_store_transaction found no active transaction running");
			trns = t_owner.get();
		}
		else trns = (store).default_connection->current_trans;
		return make_pair(move(t_owner),trns);
			}
	
	std::pair<std::unique_ptr<SQLTransaction>, SQLTransaction*>
	enter_transaction(SQLStore_impl &store, SQLTransaction *trns){
		if (!trns){
			return enter_store_transaction(store);
		}
		else return std::make_pair(std::unique_ptr<SQLTransaction>{nullptr},trns);
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
