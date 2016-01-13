//most things are in the header.

#include "ConExpr.hpp"

namespace myria { namespace mtl {

		std::string run_ast_strong(TransactionContext *ctx, const StrongCache &, const StrongStore&, const std::string& e) {
			return e;
		}

		std::string run_ast_causal(TransactionContext *ctx, const CausalCache &, const CausalStore&, const std::string& e) {
			return e;
		}

	} }
