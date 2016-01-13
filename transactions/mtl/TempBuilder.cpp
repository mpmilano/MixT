#include "TempBuilder.hpp"

namespace myria { namespace mtl {

		bool strongc_helper(TransactionContext *ctx, StrongCache &, StrongStore &, const std::shared_ptr<const std::nullptr_t>&){
			return true;
		}



		bool causalc_helper(TransactionContext *ctx, CausalCache &, CausalStore &, const std::shared_ptr<const std::nullptr_t>&){
			return true;
		}

		std::tuple<> gt_handles(std::nullptr_t const * const){
			assert(false && "cannot retrieve handles, replacement failed!");
		}

		VarScopeEnd end_var_scope() {
			static VarScopeEnd vse;
			return vse;
		}

	} }
