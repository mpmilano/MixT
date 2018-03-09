#pragma once
#include "../mutils/macro_utils.hpp"
//#include "../mutils/CTString_macro.hpp"

#define TRANSACTION(x...) ::myria::mtl::pre_transaction_str<DECT(::mutils::String<'{'>::append(MUTILS_STRING(x){}).template append<'}'>())>

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

#define TRANSACTION_FUNCTION_TRANSACTION(x...) with_operand_right<TRANSACTION(x)>{}).run_local(ct,dsm,arg);                                                    \
  }
#define TRANSACTION_FUNCTION_ARGUMENT(_this)                                                                                                                   \
  (CT & ct, DSM * dsm, const T& _this)                                                                                                                         \
  {                                                                                                                                                            \
    const auto& arg = _this;                                                                                                                                   \
    TRANSACTION_FUNCTION_WITH(_this) TRANSACTION_FUNCTION_TRANSACTION

#define TRANSACTION_METHOD_WITH2(name, ref) return (with_pre_operand_left:: template with<::myria::mtl::value_with_stringname<DECT(ref), name>>() +
#define METHOD_CPTR(ref,name) ::myria::mtl::value_with_stringname<DECT(ref), name>
#define TRANSACTION_METHOD_WITH6(name1, ref1,name2,ref2,name3,ref3) return (with_pre_operand_left:: template with<METHOD_CPTR(ref1,name1),METHOD_CPTR(ref2,name2),METHOD_CPTR(ref3,name3)>() +


#define TRANSACTION_METHOD_TRANSACTION(x...) with_operand_right<TRANSACTION(x)>{}).run_local(ct,dsm,arg1,capture1,capture2);                                                      \
  }                                                                                                                                                            \
  }                                                                                                                                                            \
  ;                                                                                                                                                            \
  return transaction_method_context::invoke(ct,dsm,arg1,transaction_method_context::capture1(this),transaction_method_context::capture2(this));                                                                                                                 \
  }

#define mixt_captures(a, b)                                                                                                                                    \
  struct transaction_method_context                                                                                                                            \
  {                                                                                                                                                            \
    using this_t = DECT(this);                                                                                                                                 \
    static const auto& capture1(this_t _this) { return _this->a; }                                                                                              \
    static const auto& capture2(this_t _this) { return _this->b; }                                                                                              \
    static auto invoke(CT& ct, DSM* dsm, const T& arg1, const DECT(a) & a, const DECT(b) & b)                                                                  \
    {                                                                                                                                                          \
      const auto& capture1 = a;                                                                                                                                \
      const auto& capture2 = b;                                                                                                                                \
      TRANSACTION_METHOD_WITH6(arg1_string, arg1,MUTILS_STRING(a),a,MUTILS_STRING(b),b) TRANSACTION_METHOD_TRANSACTION

#define TRANSACTION_METHOD_ARGUMENT(_this)                                                                                                                     \
  (CT & ct, DSM * dsm, const T& _this)                                                                                                                         \
  {                                                                                                                                                            \
    const auto& arg1 = _this;                                                                                                                                  \
    using arg1_string = MUTILS_STRING(_this);

#define mixt_function(x)                                                                                                                                       \
  template <typename CT, typename DSM, typename T>                                                                                                             \
  auto x TRANSACTION_FUNCTION_ARGUMENT
#define mixt_method(x)                                                                                                                                         \
  template <typename CT, typename DSM, typename T>                                                                                                             \
  auto x TRANSACTION_METHOD_ARGUMENT
