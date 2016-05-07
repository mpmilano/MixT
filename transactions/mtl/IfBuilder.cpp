#include "IfBuilder.hpp"

namespace myria { namespace mtl {

		const IfEnd& make_if_end(){
			static constexpr IfEnd e;
			return e;
		}
}}
