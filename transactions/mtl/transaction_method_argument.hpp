#pragma once
#include "transaction_method_transaction.hpp"
#include "transaction_method_with.hpp"

#define TRANSACTION_METHOD_ARGUMENT_LOCAL1(_this)                                                                                                              \
  (CT & ct, DSM * dsm, const auto& _this)                                                                                                                      \
  {                                                                                                                                                            \
    const auto& arg1 = _this;                                                                                                                                  \
    using arg1_string = MUTILS_STRING(_this);                                                                                                                  \
    const int arg2{ 0 };                                                                                                                                       \
    using arg2_string = MUTILS_STRING(UNUSED_POSITION_ARG2);

#define TRANSACTION_METHOD_ARGUMENT_LOCAL2(_arg1, _arg2)                                                                                                       \
  (CT & ct, DSM * dsm, const auto& _arg1, const auto& _arg2)                                                                                                   \
  {                                                                                                                                                            \
    const auto& arg1 = _arg1;                                                                                                                                  \
    const auto& arg2 = _arg2;                                                                                                                                  \
    using arg1_string = MUTILS_STRING(_arg1);                                                                                                                  \
    using arg2_string = MUTILS_STRING(_arg2);

#define TRANSACTION_METHOD_ARGUMENT_LOCAL_IMPL2(count, ...) TRANSACTION_METHOD_ARGUMENT_LOCAL##count(__VA_ARGS__)
#define TRANSACTION_METHOD_ARGUMENT_LOCAL_IMPL(count, ...) TRANSACTION_METHOD_ARGUMENT_LOCAL_IMPL2(count, __VA_ARGS__)
#define TRANSACTION_METHOD_ARGUMENT_LOCAL(...) TRANSACTION_METHOD_ARGUMENT_LOCAL_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)

#define TRANSACTION_METHOD_ARGUMENT1(_arg1)                                                                                                                    \
  (CT & ct, DSM * dsm, Handle<label, Class_t, operations...> _this_handle, const auto& _arg1)                                                                  \
  {                                                                                                                                                            \
    const auto& arg1 = _arg1;                                                                                                                                  \
    using arg1_string = MUTILS_STRING(_arg1);                                                                                                                  \
    using this_str = MUTILS_STRING(this);                                                                                                                      \
    TRANSACTION_METHOD_WITH4(arg1_string, arg1, this_str, _this_handle) TRANSACTION_METHOD_TRANSACTION1

#define TRANSACTION_METHOD_ARGUMENT_IMPL2(count, ...) TRANSACTION_METHOD_ARGUMENT##count(__VA_ARGS__)
#define TRANSACTION_METHOD_ARGUMENT_IMPL(count, ...) TRANSACTION_METHOD_ARGUMENT_IMPL2(count, __VA_ARGS__)
#define TRANSACTION_METHOD_ARGUMENT(...) TRANSACTION_METHOD_ARGUMENT_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)
