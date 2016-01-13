#include "TransactionBasics.hpp"
#include "Transaction.hpp"

namespace myria { namespace mtl {

		bool TransactionContext::full_commit() {
			if (strongContext){
				try {
					if (!strongContext->store_commit()){
						throw Transaction::StrongFailureError();
					}
				} catch (...){
					throw Transaction::StrongFailureError();
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
