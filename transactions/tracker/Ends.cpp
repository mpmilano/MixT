#include "Ends.hpp"

namespace myria { namespace tracker { 


		namespace ends{
			std::array<unsigned long long,4> max(const std::array<unsigned long long,4> &a,const std::array<unsigned long long,4> &b){
				return std::array<unsigned long long,4> {{std::max(a[0],b[0]),std::max(a[1],b[1]),std::max(a[2],b[2]),std::max(a[3],b[3])}};
			}
		}
	}}
