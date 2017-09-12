#pragma once
#include "mtl/CTString.hpp"
#include "mutils/macro_utils.hpp"

namespace mutils {

template <typename T>
class is_struct
{
  typedef char one;
  typedef long two;

  // only requirement of struct is a member called "field"
  template <typename C>
  static one test(decltype(&C::is_struct));
  template <typename C>
  static two test(...);

public:
  enum
  {
    value = sizeof(test<T>(0)) == sizeof(char)
  };
};

#define STRUCT6(type1, field1, type2, field2, type3, field3)                                                                                                   \
  struct                                                                                                                                                       \
  {                                                                                                                                                            \
    bool is_struct{ true };                                                                                                                                    \
    type1 field1;                                                                                                                                              \
    type2 field2;                                                                                                                                              \
    type3 field3;                                                                                                                                              \
    auto& field(MUTILS_STRING(field1)) { return field1; }                                                                                                      \
    auto& field(MUTILS_STRING(field2)) { return field2; }                                                                                                      \
    auto& field(MUTILS_STRING(field3)) { return field3; }                                                                                                      \
  }

#define STRUCT2(type1, field1)                                                                                                                                 \
  struct                                                                                                                                                       \
  {                                                                                                                                                            \
    bool is_struct{ true };                                                                                                                                    \
    type1 field1;                                                                                                                                              \
    auto& field(MUTILS_STRING(field1)) { return field1; }                                                                                                      \
  }

#define STRUCT_IMPL2(count, ...) STRUCT##count(__VA_ARGS__)
#define STRUCT_IMPL(count, ...) STRUCT_IMPL2(count, __VA_ARGS__)
#define STRUCT(...) STRUCT_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)
}
