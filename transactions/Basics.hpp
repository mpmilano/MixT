#pragma once
#include <string>
#include "ObjectBuilder.hpp"

namespace myria{
  
  
  constexpr int num_processes = 50;
  static_assert(num_processes <= 100,"Error: you are at risk of too many open files");
  



}
