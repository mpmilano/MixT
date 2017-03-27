#include "SQLLevels.hpp"

namespace myria {
			
	constexpr char Label<pgsql::strong>::description[];
	constexpr char Label<pgsql::causal>::description[];

	std::ostream& operator<<(std::ostream& o, const Label<pgsql::strong>&){
		return o << Label<pgsql::strong>::description;
	}
	std::ostream& operator<<(std::ostream& o, const Label<pgsql::causal>&){
		return o << Label<pgsql::causal>::description;
	}
}
