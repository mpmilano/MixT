#pragma once
#include "transaction_method_transaction.hpp"
#include "transaction_method_with.hpp"

#define mixt_captures(a, b)                                                                                                                                    \
  using this_t = DECT(this);                                                                                                                                   \
  struct transaction_method_context                                                                                                                            \
  {                                                                                                                                                            \
    static const auto& capture1(this_t _this) { return _this->a; }                                                                                             \
    static const auto& capture2(this_t _this) { return _this->b; }                                                                                             \
    static auto invoke(CT& ct, DSM* dsm, const DECT(arg1)& arg1, const DECT(a) & a, const DECT(b) & b) \
    {                                                                                                                                                          \
      const auto& capture1 = a;                                                                                                                                \
      const auto& capture2 = b;                                                                                                                                \
      using capture1_name = MUTILS_STRING(a);                                                                                                                  \
      using capture2_name = MUTILS_STRING(b);                                                                                                                  \
      TRANSACTION_METHOD_WITH6(arg1_string, arg1, capture1_name, a, capture2_name, b) TRANSACTION_METHOD_TRANSACTION

