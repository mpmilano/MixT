//for things where it's irritating to put them before other things.
#pragma once

#include "Ostreams.hpp"

template<typename T>
RemoteObject<T>* RemoteObject<T>::from_bytes(char* _v)
{
	int read_id = ((int*)_v)[0];
	char *v = _v + sizeof(int);
	typedef std::tuple<STORE_LIST> stores;
	typedef fold_types<Pointerize,stores,std::tuple<> > ptr_stores;
	ptr_stores lst;
	RemoteObject<T>* ret =
		fold(lst,[&](const auto &e, const auto &acc) -> RemoteObject<T>* {
				using DS = decay<decltype(*e)>;
				if (read_id == DS::id::value) {
					assert(!acc);
					auto *fold_ret = DS::template from_bytes<T>(v);
					assert(fold_ret);
					return fold_ret;
				}
				else return acc;
			},nullptr);
	assert(ret);
	return ret;
}

