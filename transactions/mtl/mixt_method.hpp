#pragma once
#include "mixt_captures.hpp"
#include "transaction_macros.hpp"
#include "transaction_method_argument.hpp"
#include "transaction_method_with.hpp"

#define mixt_method(x)                                                                                                                                         \
  template <typename CT, typename DSM>                                                                                                                         \
  auto x TRANSACTION_METHOD_ARGUMENT_LOCAL

#define mixt_remote_method(x)                                                                                                                                   \
  template <typename CT, typename DSM, typename label, typename Class_t, typename... operations>                                                               \
  static auto x TRANSACTION_METHOD_ARGUMENT
