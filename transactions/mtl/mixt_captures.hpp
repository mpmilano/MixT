#pragma once
#include "transaction_method_transaction.hpp"
#include "transaction_method_with.hpp"

#define mixt_captures1(a)                                                                                                                                    \
  using this_t = DECT(this);                                                                                                                                   \
  struct transaction_method_context                                                                                                                            \
  {                                                                                                                                                            \
    static const auto& capture1(this_t _this) { return _this->a; }                                                                                             \
        static auto invoke(CT& ct, DSM* dsm, const DECT(arg1) & arg1, const DECT(arg2) & arg2, const DECT(a) & a)                                                        \
    {                                                                                                                                                          \
      const auto& capture1 = a;                                                                                                                                \
      using capture1_name = MUTILS_STRING(a);                                                                                                                  \
      TRANSACTION_METHOD_WITH6(arg1_string, arg1, arg2_string,arg2,capture1_name, a, ) TRANSACTION_METHOD_TRANSACTION_LOCAL

#define mixt_captures2(a, b)                                                                                                                                    \
  using this_t = DECT(this);                                                                                                                                   \
  struct transaction_method_context                                                                                                                            \
  {                                                                                                                                                            \
    static const auto& capture1(this_t _this) { return _this->a; }                                                                                             \
    static const auto& capture2(this_t _this) { return _this->b; }                                                                                             \
    static auto invoke(CT& ct, DSM* dsm, const DECT(arg1) & arg1, const DECT(arg2) & arg2, const DECT(a) & a, const DECT(b) & b)                                                        \
    {                                                                                                                                                          \
      const auto& capture1 = a;                                                                                                                                \
      const auto& capture2 = b;                                                                                                                                \
      using capture1_name = MUTILS_STRING(a);                                                                                                                  \
      using capture2_name = MUTILS_STRING(b);                                                                                                                  \
      TRANSACTION_METHOD_WITH8(arg1_string, arg1, arg2_string,arg2,capture1_name, a, capture2_name, b) TRANSACTION_METHOD_TRANSACTION_LOCAL

#define mixt_captures_IMPL2(count, ...) mixt_captures##count(__VA_ARGS__)
#define mixt_captures_IMPL(count, ...) mixt_captures_IMPL2(count, __VA_ARGS__)
#define mixt_captures(...) mixt_captures_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)
