#pragma once
#include "../mutils/macro_utils.hpp"
#include "../mtl/mixt_method.hpp"
//#include "../mutils/CTString_macro.hpp"

#define TRANSACTION(x...) []() constexpr { \
  struct wrapper {                                                             \
    constexpr wrapper() {}                                                     \
    const char str[::mutils::cstring::str_len(#x) + 1]{#x};                    \
  };\
  return ::myria::mtl::pre_transaction_str<wrapper>{};\
}()

#define WITH1(x) template with<::myria::mtl::value_with_stringname<DECT(x), MUTILS_STRING(x)>>()
#define WITH2(x, y)                                                                                                                                            \
  template with<::myria::mtl::value_with_stringname<DECT(x), MUTILS_STRING(x)>, ::myria::mtl::value_with_stringname<DECT(y), MUTILS_STRING(y)>>()
#define WITH3(x, y, z)                                                                                                                                         \
  template with<::myria::mtl::value_with_stringname<DECT(x), MUTILS_STRING(x)>, ::myria::mtl::value_with_stringname<DECT(y), MUTILS_STRING(y)>,                \
                ::myria::mtl::value_with_stringname<DECT(z), MUTILS_STRING(z)>>()
#define WITH4(x, y, z, a)                                                                                                                                      \
  template with<::myria::mtl::value_with_stringname<DECT(x), MUTILS_STRING(x)>, ::myria::mtl::value_with_stringname<DECT(y), MUTILS_STRING(y)>,                \
                ::myria::mtl::value_with_stringname<DECT(z), MUTILS_STRING(z)>, ::myria::mtl::value_with_stringname<DECT(a), MUTILS_STRING(a)>>()

#define WITH_IMPL2(count, ...) WITH##count(__VA_ARGS__)
#define WITH_IMPL(count, ...) WITH_IMPL2(count, __VA_ARGS__)
#define WITH(...) WITH_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)

#define TYPECHECK_ONLY1(x) template typecheck_only<::myria::mtl::value_with_stringname<DECT(x), MUTILS_STRING(x)>>()
#define TYPECHECK_ONLY2(x, y)                                                                                                                                  \
  template typecheck_only<::myria::mtl::value_with_stringname<DECT(x), MUTILS_STRING(x)>, ::myria::mtl::value_with_stringname<DECT(y), MUTILS_STRING(y)>>()
#define TYPECHECK_ONLY3(x, y, z)                                                                                                                               \
  template typecheck_only<::myria::mtl::value_with_stringname<DECT(x), MUTILS_STRING(x)>, ::myria::mtl::value_with_stringname<DECT(y), MUTILS_STRING(y)>,      \
                          ::myria::mtl::value_with_stringname<DECT(z), MUTILS_STRING(z)>>()
#define TYPECHECK_ONLY4(x, y, z, a)                                                                                                                            \
  template typecheck_only<::myria::mtl::value_with_stringname<DECT(x), MUTILS_STRING(x)>, ::myria::mtl::value_with_stringname<DECT(y), MUTILS_STRING(y)>,      \
                          ::myria::mtl::value_with_stringname<DECT(z), MUTILS_STRING(z)>, ::myria::mtl::value_with_stringname<DECT(a), MUTILS_STRING(a)>>()

#define TYPECHECK_ONLY_IMPL2(count, ...) TYPECHECK_ONLY##count(__VA_ARGS__)
#define TYPECHECK_ONLY_IMPL(count, ...) TYPECHECK_ONLY_IMPL2(count, __VA_ARGS__)
#define TYPECHECK_ONLY(...) TYPECHECK_ONLY_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)

#define RUN_LOCAL_WITH(ct, dsm, a...) WITH(a).run_local(ct, dsm, a)

#define TRANSACTION_FUNCTION_WITH(x...) return (with_pre_operand_left:: WITH(x) +

#define TRANSACTION_FUNCTION_TRANSACTION(x...) with_operand_right_f(TRANSACTION(x))).run_local(ct,dsm,arg);                                                    \
  }
#define TRANSACTION_FUNCTION_ARGUMENT(_this)                                                                                                                   \
  (CT & ct, DSM * dsm, const T& _this)                                                                                                                         \
  {                                                                                                                                                            \
    const auto& arg = _this;                                                                                                                                   \
    TRANSACTION_FUNCTION_WITH(_this) TRANSACTION_FUNCTION_TRANSACTION


#define mixt_function(x)                                                                                                                                       \
  template <typename CT, typename DSM, typename T>                                                                                                             \
  auto x TRANSACTION_FUNCTION_ARGUMENT
