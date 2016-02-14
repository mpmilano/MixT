//most things are in the header.

#include "ConExpr.hpp"
#include "CommonExprs.hpp"

namespace myria { namespace mtl {

		std::string run_ast_strong(TransactionContext *ctx, const StrongCache &, const StrongStore&, const std::string& e) {
			return e;
		}

		std::string run_ast_causal(TransactionContext *ctx, const CausalCache &, const CausalStore&, const std::string& e) {
			return e;
		}

		CSConstant<Level::undef,std::string> wrap_constants(const std::string &s){
			return CSConstant<Level::undef,std::string>{s};
		}

	} }
