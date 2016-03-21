#include "TransactionBasics.hpp"
#include "Transaction.hpp"

namespace myria { namespace mtl {

		bool TransactionContext::full_commit() {
			commit_on_delete = false;
			bool abort_needed = true;
			mutils::AtScopeEnd ase{[&](){
					if (strongContext && abort_needed) strongContext->store_abort();
					if (causalContext && abort_needed) causalContext->store_abort();
				}};
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
			abort_needed = false;
			return ret;
		}

		void TransactionContext::full_abort(){
			mutils::AtScopeEnd ase{[&](){abortContext();}};
			mutils::AtScopeEnd ase2{[&](){if (strongContext) strongContext->store_abort();}};
			mutils::AtScopeEnd ase3{[&](){if (causalContext) causalContext->store_abort();}};
		}
	}
}
