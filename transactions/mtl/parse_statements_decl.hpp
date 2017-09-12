#pragma once
#include "mutils/CTString.hpp"
#include "mtl/AST_parse.hpp"

namespace myria {
namespace mtl {

template <char... str>
constexpr auto parse_statement(mutils::String<str...>);
}
}
