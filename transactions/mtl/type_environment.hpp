#pragma once

#include "mutils/CTString.hpp"
#include "mtl/mtlutils.hpp"
#include "mtl/top.hpp"
#include <cassert>
#include <iostream>
#include <type_traits>
#include <vector>
#include <array>

namespace myria {

namespace mtl {

template <char... str>
using String = mutils::String<str...>;
}
	
template <typename>
struct Label;

namespace mtl {

enum class type_location
{
  local,
  remote
};
template <typename Name, typename type, typename Label, type_location>
struct type_binding;


	
}}
