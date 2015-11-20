#pragma once
#include <type_traits>
#include "Store.hpp"

namespace context{

	enum class t{
		unknown, read, write, operation, data, validity
			};

	using id = std::integral_constant<int,-2>;

	t current_context(const CausalCache& c);
	void set_context(CausalCache&c, t ctx);	
	t current_context(const StrongCache& c);
	void set_context(StrongCache&c, t ctx);
	
}
