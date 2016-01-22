#include "TransactionBasics.hpp"
#include "Transaction.hpp"

namespace myria { namespace mtl {

		bool TransactionContext::full_commit() {
			commit_on_delete = false;
			if (strongContext){
				try {
					if (!strongContext->store_commit()){
						throw StrongFailureError();
					}
				} catch (...){
					throw StrongFailureError();
				}
			}
			auto ret = (causalContext ? causalContext->store_commit(): true);
			if (ret){
				commitContext();
				committed = true;
			}
			return ret;
		}
	}
}
