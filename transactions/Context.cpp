#include "Context.hpp"

namespace context{

	t current_context(const CausalCache& c){
		return c.get<t>(id::value);
	}

	void set_context(CausalCache&c, t ctx){
		c.emplace_ovrt<t>(id::value,ctx);
	}
	
	t current_context(const StrongCache& c){
		return c.get<t>(id::value);
	}

	void set_context(StrongCache&c, t ctx){
		c.emplace_ovrt<t>(id::value,ctx);
	}
}
