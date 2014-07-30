#include "Client.h"
#include <iostream>

using namespace backend;

typedef void (*copy_hndls_f) (DataStore& from, DataStore &to);

void backend::Client::waitForSync(){
	
	static const copy_hndls_f copy_hndls = [](DataStore& from, DataStore &to){
		for (auto& ptr_copy : to.hndls) {
			auto& ptr = ptr_copy.first;
			auto& copy = ptr_copy.second;
			if (from.hndls.size() <= ptr->id) {
				auto &m_ptr = from.hndls[ptr->id].first;
				if (m_ptr->rid == ptr->rid) {
					ptr.operator=(copy(*m_ptr, to));
				}
			}
		}
	};
	copy_hndls(master,local);
	for (auto &update : pending_updates) update();
	copy_hndls(local,master);
	pending_updates.clear();
}
