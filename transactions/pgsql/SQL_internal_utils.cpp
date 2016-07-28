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
			if (store.current_transaction){
				return std::make_pair(std::unique_ptr<SQLTransaction>{},store.current_transaction);
			}
			else {
				auto t_owner =
				small_transaction(store,"enter_store_transaction found no active transaction running");
				auto *t_ptr = t_owner.get();
				return std::make_pair(std::move(t_owner),t_ptr);
			}
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
			if (r[0].size() < 4) std::cout << "offending Query: \"" << r.query() << "\"" << std::endl;
			assert(r[0].size() >= 4);
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
