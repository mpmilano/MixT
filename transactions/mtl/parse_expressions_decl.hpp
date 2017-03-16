#pragma once
#include "CTString.hpp"
#include "AST_parse.hpp"

namespace myria {
namespace mtl {

template <char... str>
constexpr auto parse_expression(mutils::String<str...>);
}
}
