#pragma once

#define TRANSACTION_METHOD_TRANSACTION_LOCAL2(x...) with_operand_right_f(TRANSACTION(x))); return txn.run_local(ct,dsm,arg1,arg2,capture1,capture2);                                   \
  }                                                                                                                                                            \
  }                                                                                                                                                            \
  ;                                                                                                                                                            \
  return transaction_method_context::invoke(ct, dsm, arg1,arg2, transaction_method_context::capture1(this), transaction_method_context::capture2(this));            \
  }


#define TRANSACTION_METHOD_TRANSACTION_LOCAL1(x...) with_operand_right_f(TRANSACTION(x)){});  return txn.run_local(ct,dsm,arg1,arg2,capture1);                                   \
  }                                                                                                                                                            \
  }                                                                                                                                                            \
  ;                                                                                                                                                            \
  return transaction_method_context::invoke(ct, dsm, arg1,arg2, transaction_method_context::capture1(this));            \
  }


#define TRANSACTION_METHOD_TRANSACTION1(x...) with_operand_right_f(TRANSACTION(x))); return txn.run_remote(ct,dsm,_this_handle,arg1);                                   \
  }                                                                                                                                                            \


