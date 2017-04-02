#pragma once
#include "CTString_macro.hpp"

#define TRANSACTION(n,x...) ::myria::mtl::pre_transaction_str<n,MUTILS_STRING(x)>

#define WITH1(x) template with<::myria::mtl::value_with_stringname<DECT(x), MUTILS_STRING(x)>>()
#define WITH2(x, y)                                                                                                                                            \
  template with<::myria::mtl::value_with_stringname<DECT(x), MUTILS_STRING(x)>, ::myria::mtl::value_with_stringname<DECT(y), MUTILS_STRING(y)>>()
#define WITH3(x, y, z)                                                                                                                                         \
  template with<::myria::mtl::value_with_stringname<DECT(x), MUTILS_STRING(x)>, ::myria::mtl::value_with_stringname<DECT(y), MUTILS_STRING(y)>,                \
                ::myria::mtl::value_with_stringname<DECT(z), MUTILS_STRING(z)>>()

#define WITH_IMPL2(count, ...) WITH##count(__VA_ARGS__)
#define WITH_IMPL(count, ...) WITH_IMPL2(count, __VA_ARGS__)
#define WITH(...) WITH_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)
