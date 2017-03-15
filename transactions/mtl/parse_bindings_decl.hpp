#pragma once
#include "CTString.hpp"

namespace myria {
namespace mtl {

template <char... str>
constexpr auto parse_binding(mutils::String<str...>);
}
}
