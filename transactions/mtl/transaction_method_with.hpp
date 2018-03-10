#pragma once

#define TRANSACTION_METHOD_WITH2(name, ref) constexpr auto txn = (with_pre_operand_left<::myria::mtl::value_with_stringname<DECT(ref), name>>{} +
#define METHOD_CPTR(ref, name) ::myria::mtl::value_with_stringname<DECT(ref), name>
#define TRANSACTION_METHOD_WITH6(name1, ref1, name2, ref2, name3, ref3) constexpr auto txn =(with_pre_operand_left<METHOD_CPTR(ref1,name1),METHOD_CPTR(ref2,name2),METHOD_CPTR(ref3,name3)>{} +
#define TRANSACTION_METHOD_WITH8(name1, ref1, name2, ref2, name3, ref3,name4,ref4) constexpr auto txn = (with_pre_operand_left<METHOD_CPTR(ref1,name1),METHOD_CPTR(ref2,name2),METHOD_CPTR(ref3,name3),METHOD_CPTR(ref4,name4)>{} +
#define TRANSACTION_METHOD_WITH4(name1, ref1, name2, ref2) constexpr auto txn = (with_pre_operand_left<METHOD_CPTR(ref1,name1),METHOD_CPTR(ref2,name2)>{} +
