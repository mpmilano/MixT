#include "mailing_list_example.hpp"
#include "mtl/typecheck_printer.hpp"
#include "mtl/split_printer.hpp"
#include <iostream>

namespace examples{
	std::list<message> download_inbox(client<mailing_list_state>& ct, user_hndl user_hndl){
		assert(user_hndl._ro);
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
				).WITH(user_hndl);
		txn.print();
#endif
		using connections = typename DECT(ct.trk)::connection_references;
		auto strong_connection = ct.get_relay<Level::strong>().lock();
		auto causal_connection = ct.get_relay<Level::causal>().lock();
		return txn.run_remote(ct.trk,&ct.dsm,
											 connections{ConnectionReference<Label<strong> >{*strong_connection},ConnectionReference<Label<causal> >{*causal_connection}},user_hndl);
	}
}
