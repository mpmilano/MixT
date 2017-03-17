#include "Ostreams.hpp"

namespace myria{

	using namespace mtl;

	auto print_util(const std::shared_ptr<const std::nullptr_t>&){
		return "aaaaaa";
	}
	
	namespace tracker {
		std::ostream& operator<<(std::ostream& os, const Tracker::Tombstone &){
			return os << "tombstone";
		}
	}
}
