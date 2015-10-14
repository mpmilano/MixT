//most things are in the header.

#include "ConExpr.hpp"

std::string run_ast_strong(const StrongCache &, const StrongStore&, const std::string& e) {
	return e;
}

std::string run_ast_causal(const CausalCache &, const CausalStore&, const std::string& e) {
	return e;
}
