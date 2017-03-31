#pragma once

#include "OptimisticStoreWrapper.hpp"

namespace optimism{

	template<typename Store, typename... Operations>
	struct OptimisticContext : public mtl::StoreContext<typename Store::label> {
		using Parent = OptimisticStore<Store,Operations...>;
		Parent &parent;

		OptimisticContext(Parent &parent)
			:parent(parent){}
			
		using phases = typename Parent::phases;
		phases phase{phases::collect_phase};		
		std::set<GeneralRemoteObject* > read_set;
		std::set<GeneralRemoteObject* > write_set;
		HeteroMap<GeneralRemoteObject*, shared_ptr_alias> working_set;
		
		void mark_needed(GeneralRemoteObject& n){
			cache_read_lock lock{parent.cache_lock};
			if (!parent.cache().contains(&n) && !working_set.contains(&n))
				read_set.insert(&n);
		}
		
		void mark_written(GeneralRemoteObject& n){
			cache_read_lock lock{parent.cache_lock};
			write_set.insert(&n);
		}
		
		//cache entry points
		bool contains(GeneralRemoteObject& n){
			switch(phase){
			case phases::collect_phase:
				mark_needed(n);
				cache_read_lock lock{parent.cache_lock};
				return parent.cache().contains(n);
			};
		}
		
		template<typename T>
		bool contains_valid(GeneralRemoteObject& n){
			return contains(n) && get(n);
		}
		
		template<typename T> std::shared_ptr<const T> get(GeneralRemoteObject& n){
			switch(phase){
			case phases::collect_phase:
				mark_needed(n);						
				if (working_set.contains(&n)){
					return working_set.template at<T>(&n);
				}
				cache_read_lock lock{parent.cache_lock};
				if (parent.cache().contains(&n)) return parent.cache().template at<T>(&n);
				else return std::make_shared<const T>();
			};
		}
		
		template<typename T> void put(GeneralRemoteObject&, const T& t){
			switch(phase){
			case phases::collect_phase:
				mark_written(n);
				(*working_set.template mut<T>(&n)) = t;
				return;
			};
		}
	};
}
