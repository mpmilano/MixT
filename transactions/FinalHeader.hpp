//for things where it's irritating to put them before other things.
#pragma once

template<typename T>
RemoteObject<T>* RemoteObject<T>::from_bytes(char* _v)
{
	int read_id = ((int*)_v)[0];
	char *v = _v + sizeof(int);
	typedef std::tuple<STORE_LIST> stores;
	typedef fold_types<Pointerize,stores,std::tuple<> > ptr_stores;
	ptr_stores lst;
	return fold(lst,[&](const auto &e, const auto &acc) -> RemoteObject<T>* {
			using DS = decay<decltype(*e)>;
			if (read_id == DS::id::value) return DS::template from_bytes<T>(v);
				else return acc;
		},nullptr);
}
