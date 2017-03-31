#pragma once

#include "OptimisticStoreWrapper.hpp"

namespace optimism {

	
	template<typename T, typename Store, typename... Operations>
	struct OptimisticObject : public RemoteObject<label,T> {
		using Parent = OptimisticStore<Store,Operations...>;
		
		Parent& _store;
		
		typename Parent::Handle hdnl;
		bool ro_isValid(mtl::StoreContext<label>* _ctx) const {
			auto& ctx = dynamic_cast<StoreContext&>(*_ctx);
			return hndl._ro && ctx.template contains_valid<T>(name);
		}
		
		std::shared_ptr<const T> get(mtl::StoreContext<l>* _ctx) {
			auto& ctx = dynamic_cast<StoreContext&>(*_ctx);
			return ctx.template get<T>(name);
		}
		
		void put(mtl::StoreContext<l>* _ctx,const T& t) {
			auto& ctx = dynamic_cast<StoreContext&>(*_ctx);
			return ctx.put(name);
		}
		
		const DataStore<level>& store() const {
			return _store;
		}
		DataStore<level>& store() {
			return _store;
		}
		
	};

	template<typename T, typename Store, typename... Operations>
	StoreObject<T>& unwrap_ro(OptimisticObject<T>& o){
		return dynamic_cast<typename Store::template StoreObject<T>& >(o.hndl.remote_object());
	}
	
	template<typename T, typename Store, typename... Operations>
	T unwrap_ro(T&& o){
		return o;
	}
	
	template<typename T, typename Store, typename... Operations>
	std::shared_ptr<const T> deref_ro(OptimisticContext<Store,Operations...>& ctx,
																		OptimisticObject<T>& o){
		return o.get(&ctx);
	}
	
	template<typename T, typename Store, typename... Operations>
	T deref_ro(OptimisticContext<Store,Operations...>& ctx, T&& o){
		return o;
	}
}
