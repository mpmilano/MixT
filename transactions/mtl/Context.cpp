#include "Context.hpp"

namespace myria { namespace mtl {

namespace context{

	t current_context(const CausalCache& c){
		return c.get<t>(id::value);
	}

	void set_context(CausalCache&c, t ctx){
		c.emplace_ovrt<t>(id::value,ctx);
        assert(c.contains(id::value));
	}
	
	t current_context(const StrongCache& c){
		assert(c.contains(id::value) && "Error: this cache has not been seeded with the context");
		return c.get<t>(id::value);
	}

	void set_context(StrongCache&c, t ctx){
		c.emplace_ovrt<t>(id::value,ctx);
	}
}

	} }
