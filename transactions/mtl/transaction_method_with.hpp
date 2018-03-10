#pragma once

#define TRANSACTION_METHOD_WITH2(name, ref) return (with_pre_operand_left<::myria::mtl::value_with_stringname<DECT(ref), name>>{} +
#define METHOD_CPTR(ref, name) ::myria::mtl::value_with_stringname<DECT(ref), name>
#define TRANSACTION_METHOD_WITH6(name1, ref1, name2, ref2, name3, ref3) return (with_pre_operand_left<METHOD_CPTR(ref1,name1),METHOD_CPTR(ref2,name2),METHOD_CPTR(ref3,name3)>{} +

