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

using true_binding = type_binding<mutils::String<'t','r','u','e'>, bool, Label<top>,type_location::local>;
using false_binding = type_binding<mutils::String<'f','a','l','s','e'>, bool, Label<top>,type_location::local>;
	
}}
