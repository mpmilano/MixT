#pragma once

namespace mutils {
struct zero
{
};
template <typename>
struct succ;
template <typename prev>
struct succ<succ<prev>>
{
};
template <>
struct succ<zero>
{
};
using one = succ<zero>;
}
