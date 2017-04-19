#pragma once
#include <algorithm>
#include <array>

namespace myria { namespace tracker { 

		namespace ends{
			std::array<long long,4> max(const std::array<long long,4> &a,const std::array<long long,4> &b);

			template<typename... Arrays>
			std::array<long long,4> max(const std::array<long long,4> &a,
																	const std::array<long long,4> &b,
																	const std::array<long long,4> &c,
																	const Arrays& ... arrays){
				return max(a,max(b,c, arrays...));
			}

			template<typename Iterator>
			std::array<long long, 4> max(Iterator begin, Iterator end){
				std::array<long long, 4> ret{{-1,-1,-1,-1}};
				for (auto it = begin; it != end; ++it){
					ret = max(ret,*it);
				}
				return ret;
			}
	
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
