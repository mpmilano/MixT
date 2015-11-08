//for things where it's irritating to put them before other things.
#pragma once

template<Level l, typename T>
std::shared_ptr<RemoteObject<l,T> > release_delete(RemoteObject<l,T>* ro){
	return std::shared_ptr<RemoteObject<l,T> >(ro, release_deleter<RemoteObject<l,T> >());
}

template<Level l1, Level l2, typename T, restrict(l1 != l2)>
std::shared_ptr<RemoteObject<l1,T> > release_delete(RemoteObject<l2,T>* ){
	assert(false && "this should be unreachable");
}

template<Level l, typename T>
std::unique_ptr<RemoteObject<l,T> > RemoteObject<l,T>::from_bytes(char* _v)
{
	int read_id = ((int*)_v)[0];
	char *v = _v + sizeof(int);
	typedef std::tuple<STORE_LIST> stores;
	typedef fold_types<Pointerize,stores,std::tuple<> > ptr_stores;
	ptr_stores lst;
	auto ret = 
		fold(lst,[&](const auto &e, const std::shared_ptr<RemoteObject<l,T> > &acc) -> std::shared_ptr<RemoteObject<l,T> > {
				using DS = decay<decltype(*e)>;
				if (read_id == DS::id()) {
					assert(!acc);
					auto fold_ret = DS::template from_bytes<T>(v);
					assert(fold_ret);
					return release_delete<l>(fold_ret.release());
				}
				else return acc;
			},nullptr);
	if (!ret) std::cerr << "Error: deserialized item matches no id: "
						<< read_id << std::endl;
	assert(ret);
	assert(ret.unique());
	std::get_deleter<release_deleter<RemoteObject<l,T> > >(ret)->release();
	return std::unique_ptr<RemoteObject<l,T> >{ret.get()};
}


//TODO: there should be a way to get rid of these.
