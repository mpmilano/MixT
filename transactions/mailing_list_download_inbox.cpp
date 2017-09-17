#include "mailing_list_example.hpp"
#include "mtl/typecheck_printer.hpp"
#include "mtl/split_printer.hpp"
#include <iostream>

namespace examples{
	std::list<message> download_inbox(ClientTrk& ct, user_hndl user_hndl){
#ifdef USE_PRECOMPILED
		constexpr 
#include "mailing_list_download_inbox.cpp.precompiled"
			txn;
#else
		constexpr auto txn = 
			TRANSACTION(
				var lst = default list,
				var curr_msg_ptr = user_hndl->i,
				while (curr_msg_ptr.isValid()){
					lst.push_back(*curr_msg_ptr->value),
					remote mutable = curr_msg_ptr,
					curr_msg_ptr = curr_msg_ptr->next,
					mutable.next = mutable.next.nulled()
				},
				return lst
				)::WITH(user_hndl);
#endif
		std::cout << txn << std::endl;
		return txn.run_local(ct,i);
	}
}
