#include "top.hpp"

namespace myria{

std::ostream& operator<<(std::ostream& o, const Label<top>&)
{
  return o << "top";
}

std::ostream& operator<<(std::ostream& o, const Label<bottom>&)
{
  return o << "bottom";
}

}
