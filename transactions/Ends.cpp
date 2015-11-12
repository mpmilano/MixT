#include "Ends.hpp"


namespace ends{
	std::array<int,4> max(const std::array<int,4> &a,const std::array<int,4> &b){
		return std::array<int,4> {{std::max(a[0],b[0]),std::max(a[1],b[1]),std::max(a[2],b[2]),std::max(a[3],b[3])}};
	}
}
