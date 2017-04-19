#pragma once
#include <algorithm>
#include <array>

namespace myria { namespace tracker { 

		namespace ends{
			std::array<long long,4> max(const std::array<long long,4> &a,const std::array<long long,4> &b);
	
			template<size_t s>
			bool is_same(const std::array<long long,s> &a,const std::array<long long,s> &b){
                                for (decltype(s) i = 0; i < s; ++i){
					if (a[i] != b[i]) return false;
				}
				return true;
			}
	
			template<size_t s>
			bool prec(const std::array<long long,s> &a,const std::array<long long,s> &b){
                                for (decltype(s) i = 0; i < s; ++i){
					assert(a[i] != -1);
					assert(b[i] != -1);
					if (a[i] > b[i]) return false;
				}
				return true;
			}
			template<size_t s>
			bool operator<(const std::array<long long,s> &a,const std::array<long long,s> &b){
				return prec(a,b);
			}
		}
	}}
