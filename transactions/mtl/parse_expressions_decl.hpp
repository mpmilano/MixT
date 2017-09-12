#pragma once
#include "mtl/CTString.hpp"
#include "mtl/AST_parse.hpp"

namespace myria {
namespace mtl {

template <char... str>
constexpr auto parse_expression(mutils::String<str...>);
}
}
