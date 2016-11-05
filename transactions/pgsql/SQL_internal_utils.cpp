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
		enter_store_transaction(SQLStore_impl& store){
			unique_ptr<SQLTransaction> t_owner;
			SQLTransaction *trns = nullptr;
			if (!(store).in_transaction()){
				t_owner = small_transaction(store whendebug(,"enter_store_transaction found no active transaction running"));
				trns = t_owner.get();
			}
			else trns = (store).default_connection.lock()->current_trans;
			return make_pair(move(t_owner),trns);
		}
		
		std::pair<std::unique_ptr<SQLTransaction>, SQLTransaction*>
		enter_transaction(SQLStore_impl &store, SQLTransaction *trns){
			if (!trns){
				return enter_store_transaction(store);
			}
			else return make_pair(unique_ptr<SQLTransaction>{nullptr},trns);
		}
	}
}
