#pragma once
#include <string>

namespace myria{
  
  
  constexpr int num_processes = 50;
  static_assert(num_processes <= 100,"Error: you are at risk of too many open files");
  
	using Name = long int;



}
