#pragma once
#include <algorithm>
#include <array>

namespace myria { namespace tracker { 

    namespace ends{
      std::array<int,4> max(const std::array<int,4> &a,const std::array<int,4> &b);
	
      template<size_t s>
      bool is_same(const std::array<int,s> &a,const std::array<int,s> &b){
	for (int i = 0; i < s; ++i){
	  if (a[i] != b[i]) return false;
	}
	return true;
      }
	
      template<size_t s>
      bool prec(const std::array<int,s> &a,const std::array<int,s> &b){
	for (int i = 0; i < s; ++i){
	  assert(a[i] != -1);
	  assert(b[i] != -1);
	  if (a[i] > b[i]) return false;
	}
	return true;
      }
    }
  }}
