#include "TempBuilder.hpp"

namespace myria { namespace mtl {

                bool strongc_helper(TransactionContext *, StrongCache &, StrongStore &, const std::shared_ptr<const std::nullptr_t>&){
			return true;
		}



                bool causalc_helper(TransactionContext *, CausalCache &, CausalStore &, const std::shared_ptr<const std::nullptr_t>&){
			return true;
		}

		std::tuple<> gt_environment_expressions(std::nullptr_t const * const){
			assert(false && "cannot retrieve handles, replacement failed!");
			struct dead_code{}; throw dead_code{};
		}

		VarScopeEnd end_var_scope() {
			return VarScopeEnd{};
		}

	} }
