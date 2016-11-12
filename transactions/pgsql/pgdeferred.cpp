#include "LocalSQLConnection.hpp"
#include "pgtransaction.hpp"
#include "pgtransaction_impl.hpp"

namespace myria { namespace pgsql {
		namespace local{

			void deferred_transaction::indicate_no_future_actions(){
				if (trans){
					trans->indicate_no_future_actions();
				}
				else no_fut_actions = true;
			}

			bool deferred_transaction::no_future_actions() const {
				return no_fut_actions;
			}

			deferred_transaction::deferred_transaction(pgtransaction& trans)
				:trans(&trans){}
			
			deferred_transaction::~deferred_transaction(){
				if (trans){
					trans->my_trans = nullptr;
				}
			}

		}}}
