#include "SQLConstants.hpp"

namespace myria {
	namespace pgsql {
		
		std::ostream& operator<<(std::ostream& o, Level l){
			switch(l){
			case Level::strong:
				return o << "strong";
			case Level::causal:
				return o << "causal";
			}
		}

	}}
