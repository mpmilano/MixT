#pragma once

#define TRANSACTION_METHOD_TRANSACTION(x...) with_operand_right<TRANSACTION(x)>{}).run_local(ct,dsm,arg1,capture1,capture2);                                   \
  }                                                                                                                                                            \
  }                                                                                                                                                            \
  ;                                                                                                                                                            \
  return transaction_method_context::invoke(ct, dsm, arg1, transaction_method_context::capture1(this), transaction_method_context::capture2(this));            \
  }

