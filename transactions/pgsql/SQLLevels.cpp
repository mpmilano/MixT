#include "pgsql/SQLLevels.hpp"

namespace myria {

using namespace pgsql;

std::ostream &operator<<(std::ostream &o, const Label<strong> &) {
  return o << Label<strong>::description{};
}
std::ostream &operator<<(std::ostream &o, const Label<causal> &) {
  return o << Label<causal>::description{};
}
namespace pgsql {
std::ostream &operator<<(std::ostream &o, const Level &l) {
  if (l == Level::causal) {
    return o << Label<causal>{};
  } else {
    assert(l == Level::strong);
    return o << Label<strong>{};
  }
}
}
}
