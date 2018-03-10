#pragma once

#define TRANSACTION_METHOD_ARGUMENT(_this)                                                                                                                     \
  (CT & ct, DSM * dsm, const auto& _this)                                                                                                                      \
  {                                                                                                                                                            \
    const auto& arg1 = _this;                                                                                                                                  \
    using arg1_string = MUTILS_STRING(_this);


