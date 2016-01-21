#pragma once
#include <map>
#include <set>

namespace mutils {

	template<typename Key>
	struct HeteroMap {
	private:
		
		using type_id = int;
		using destructor = std::function<void ()>;
		template<typename T>
		using submap = std::map<Key, std::unique_ptr<T> >;
		
		static type_id type_id_counter(bool increment){
			static type_id counter = 0;
			if (increment) ++counter;
			return counter;
		}
		
		template<typename T>
		static type_id get_type_id(){
			static auto id_for_T = type_id_counter(true);
			return id_for_T;
		}
		
		std::map<type_id, std::pair<void*, destructor> > sub_maps;
		std::set<Key> member_set; //for membership queries, when we do not know the type
		
		template<typename T>
		auto* get_submap(){
			auto tid = get_type_id<T>();
			if (sub_maps.count(tid) == 0){
				submap<T>* newmap = new submap<T>{};
				sub_maps.emplace(
					tid,
					std::pair<void*,destructor>(newmap,[newmap](){delete newmap;}));
			}
			return (submap<T>*) sub_maps[tid].first;
		}
		
		template<typename T>
		auto get_submap() const {
			return (submap<T>*) sub_maps.at(get_type_id<T>()).first;
		}
	public:
		
		template<typename T>
		auto& at(Key i) const {
			auto tid = get_type_id<T>();
			assert(sub_maps.count(tid) > 0);
			return get_submap<T>()->at(i);
		}
		
		template<typename T>
		auto& mut(Key i){
			member_set.insert(i);
			return (*get_submap<T>())[i];
		}
		
		bool contains(Key i) const {
			return member_set.count(i) > 0;
		}
		
		virtual ~HeteroMap(){
			for (auto& p : sub_maps){
				p.second.second();
			}
		}
	};
}

