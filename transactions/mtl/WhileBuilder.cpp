#include "WhileBuilder.hpp"

namespace myria { namespace mtl {

		const WhileEnd& make_while_end(){
			static constexpr WhileEnd e;
			return e;
		}


	} }
