//for things where it's irritating to put them before other things.
#pragma once
#include "Handle.hpp"

namespace myria{

	template<Level l, typename T, HandleAccess HA, typename...  ops>
	std::shared_ptr<Handle<l,HA,T,ops...> > release_delete(Handle<l,HA,T,ops...>* ro){
		return std::shared_ptr<Handle<l,HA,T,ops...> >(ro, release_deleter<Handle<l,HA,T,ops...> >());
	}

	template<Level l1, Level l2, typename T, HandleAccess HA, typename...  ops>
	std::enable_if_t<l1 != l2, std::shared_ptr<Handle<l1,HA,T,ops...> > > release_delete(Handle<l2,HA,T,ops...>* ){
		assert(false && "this should be unreachable");
		struct dead_code{};
		throw dead_code{};
	}

	template<Level l, HandleAccess ha, typename T, typename... ops>
	std::unique_ptr<Handle<l,ha,T,ops...> > hndl_from_bytes(mutils::DeserializationManager* dm, char const * __v)
	{
		//using Handle = Handle<l,ha,T,ops...>;
		assert(__v);
		if (((bool*)__v)[0]) {
			auto *_v = __v + sizeof(bool);
			int read_id = ((int*)_v)[0];
			auto *v = _v + sizeof(int);
			typedef std::tuple<STORE_LIST> stores;
			typedef mutils::fold_types<mutils::Pointerize,stores,std::tuple<> > ptr_stores;
			ptr_stores lst;
			auto ret = 
				mutils::fold(lst,[&](const auto &e, const std::shared_ptr<Handle<l,ha,T,ops...> > &acc) -> std::shared_ptr<Handle<l,ha,T,ops...> > {
						using DS = std::decay_t<decltype(*e)>;
						if (read_id == DS::id()) {
							assert(!acc);
							auto fold_ret = DS::template from_bytes<ha,T>(dm,v);
							static_assert(std::is_same<typename decltype(fold_ret)::element_type::stored_type,T>::value,"Error: from_bytes error");
							assert(fold_ret);
							return release_delete<l>(fold_ret.release());
						}
						else return acc;
					},nullptr);
			if (!ret) std::cerr << "Error: deserialized item matches no id: "
								<< read_id << std::endl;
			assert(ret);
			assert(ret.unique());
			std::get_deleter<release_deleter<Handle<l,ha,T,ops...> > >(ret)->release();
			return std::unique_ptr<Handle<l,ha,T,ops...> >{ret.get()};
		}
		else return std::make_unique<Handle<l,ha,T,ops...>>();
	}


	//TODO: there should be a way to get rid of these.
}
