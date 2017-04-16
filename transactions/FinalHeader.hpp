//for things where it's irritating to put them before other things.
#pragma once
#include "Handle.hpp"
#include "UnmatchedRemoteObject.hpp"

namespace myria{

	template<typename l, typename T, typename...  ops>
	std::shared_ptr<Handle<l,T,ops...> > release_delete(Handle<l,T,ops...>* ro){
		return std::shared_ptr<Handle<l,T,ops...> >(ro, release_deleter<Handle<l,T,ops...> >());
	}

	template<typename l1, typename l2, typename T, typename...  ops>
	std::enable_if_t<!std::is_same<l1,  l2>::value, std::shared_ptr<Handle<l1,T,ops...> > > release_delete(Handle<l2,T,ops...>* ){
		assert(false && "this should be unreachable");
		struct dead_code{};
		throw dead_code{};
	}

	template<typename l, typename T, typename... ops>
	std::unique_ptr<Handle<l,T,ops...> > hndl_from_bytes(mutils::DeserializationManager* dm, char const * __v, Handle<l,T,ops...>*)
	{
		//using Handle = Handle<l,ha,T,ops...>;
		assert(__v);
		if (((bool*)__v)[0]) {
			std::size_t expected_size = *((std::size_t*)__v + sizeof(bool));
			auto *_v = __v + sizeof(bool) + sizeof(expected_size);
			int read_id = ((int*)_v)[0];
			auto *v = _v;
			typedef std::tuple<STORE_LIST> stores;
			typedef mutils::fold_types<mutils::Pointerize,stores,std::tuple<> > ptr_stores;
			ptr_stores lst;
			auto ret = 
				mutils::fold(lst,[&](const auto &e, const std::shared_ptr<Handle<l,T,ops...> > &acc) -> std::shared_ptr<Handle<l,T,ops...> > {
						using DS = std::decay_t<decltype(*e)>;
						if (read_id == DS::id()) {
							assert(!acc);
							auto fold_ret = DS::template from_bytes<T>(dm,v);
							static_assert(std::is_same<typename decltype(fold_ret)::element_type::stored_type,T>::value,"Error: from_bytes error");
							assert(fold_ret);
							return release_delete<l>(fold_ret.release());
						}
						else return acc;
					},nullptr);
			if (!ret) {
				using UnmatchedStore = UnmatchedDataStore<l,T,ops...>;
				ret = release_delete<l>(new Handle<l,T,ops...>{
						std::make_shared<typename UnmatchedStore::template UnmatchedRemoteObject<T> >(v,expected_size),
							UnmatchedStore::inst() });
			}
			assert(ret);
			assert(ret.unique());
			std::get_deleter<release_deleter<Handle<l,T,ops...> > >(ret)->release();
			return std::unique_ptr<Handle<l,T,ops...> >{ret.get()};
		}
		else return std::make_unique<Handle<l,T,ops...>>();
	}


	//TODO: there should be a way to get rid of these.
}
