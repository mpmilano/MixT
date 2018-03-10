#pragma once
#include "transaction_macros.hpp"
#include "transaction_method_with.hpp"
#include "transaction_method_argument.hpp"
#include "mixt_captures.hpp"


#define mixt_method(x)                                                                                                                                         \
  template <typename CT, typename DSM>                                                                                                                         \
  auto x TRANSACTION_METHOD_ARGUMENT
