#pragma once
#include <map>
#include <set>
#include "utils.hpp"

namespace mutils {

	template<typename Key>
	struct HeteroMap {
	private:
		
		using type_id = int;
		using count = std::function<int (Key)>;
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
		
		std::map<type_id, std::tuple<void*, count, destructor,std::string> > sub_maps;
		std::map<Key,type_id> member_set; //for membership queries, when we do not know the type
		
		template<typename T>
		submap<T>* get_submap(){
			auto tid = get_type_id<T>();
			if (sub_maps.count(tid) == 0){
				submap<T>* newmap = new submap<T>{};
				sub_maps.emplace(
					tid,
					std::tuple<void*,count,destructor,std::string>{
						newmap,
							[newmap](Key k){return newmap->count(k);},
							[newmap](){delete newmap;},
								type_name<T>()
								});
			}
			return (submap<T>*) std::get<0>(sub_maps[tid]);
		}
		
		template<typename T>
		submap<T> const * const get_submap() const {
			return (submap<T>*) std::get<0>(sub_maps.at(get_type_id<T>()));
		}
	public:
		
		template<typename T>
		auto& at(Key i) const {
			auto tid = get_type_id<T>();
			assert([&]() -> bool {
					if (sub_maps.count(tid) == 0){
						if (member_set.count(i) != 0){
							//this would indicate that there's a disagreement between the type mapping in
							//member_set and the static type_id assignment
							assert(get_type_id<T>() == get_type_id<T>());
							assert(sub_maps.count(member_set.at(i)) > 0);
							std::cerr << "we have type ("
									  << std::get<3>(sub_maps.at(member_set.at(i)))
									  << ") for this key; you asked for ("
									  << type_name<T>() << ")" << std::endl;
							assert(false && "HeteroMap failure");
						}
					}
					return true;
				}());
			//this is just a normal failure of at() when there's no element to retrieve.
			assert(sub_maps.count(tid) > 0);
			return get_submap<T>()->at(i);
		}
		
		template<typename T>
		auto& mut(Key i){
			member_set.emplace(i,get_type_id<T>());
			return (*get_submap<T>())[i];
		}
		
		bool contains(Key i) const {
			bool ret = member_set.count(i) > 0;
			assert(!ret || (ret && std::get<1>(sub_maps.at(member_set.at(i)))(i) > 0));
			return ret;
		}
		
		virtual ~HeteroMap(){
			for (auto& p : sub_maps){
				std::get<2>(p.second)();
			}
		}
	};
}

