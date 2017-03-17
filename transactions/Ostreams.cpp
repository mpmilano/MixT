#include "Ostreams.hpp"

namespace myria{

	using namespace mtl;

	namespace mtl{
		std::ostream & operator<<(std::ostream &os, const nope& ){
			return os << "nope!";
		}
	}

	auto print_util(const std::shared_ptr<const std::nullptr_t>&){
		return "aaaaaa";
	}

	namespace mtl {	

		std::ostream & operator<<(std::ostream &os, const Print_Str& op){
			return os << "print " << op.t << std::endl;
		}

	}

	
	namespace tracker {
		std::ostream& operator<<(std::ostream& os, const Tracker::Tombstone &){
			return os << "tombstone";
		}
	}
}
