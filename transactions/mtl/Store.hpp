#pragma once
#include <list>
#include "utils.hpp"

namespace myria { namespace mtl {

		struct StoreMiss : public mutils::StaticMyriaException<MACRO_GET_STR("error: Store Miss!")> {};

		enum class StoreType{
			StrongStore,
				CausalStore,
				StrongCache,
				CausalCache
				};

		constexpr bool is_store(StoreType st){
			return st == StoreType::CausalStore || st == StoreType::StrongStore;
		}


		static std::map<int, void*> store_lost_and_found_map;

		template<StoreType semantic_switch>
		struct StoreMap {
		private:
			struct internal_store{
				static int new_id(int reset_id = -1){
					static int max_id_counter = 0;
					static std::list<int> free_map_ids;
					if (reset_id != -1){
						//reset things
						free_map_ids.push_back(reset_id);
						return -1;
					}
					else 
						return
							(free_map_ids.empty() ? 
							 //take current max_id_counter, increment it
							 max_id_counter++
							 : 
							 //grab an id from the list
							 [&](){auto ret = free_map_ids.front();
								free_map_ids.pop_front();
								return ret;}());
				}
				
				template<typename T>
				static std::map<int,std::unique_ptr<T> >& type_specific_store(int id, bool del){
					static std::map<int, std::map<int,std::unique_ptr<T> > > in_use_maps;
					if (del) {
						new_id(id); //release this id
						in_use_maps[id].clear();
					}
					return in_use_maps[id];
				}

				const int store_id{new_id()};
				struct internal_store_shared{
					template<typename T>
					static const auto& at(int store_id, int id, T*) {
						return internal_store::type_specific_store<T>(store_id,false).at(id);
					}
					
					template<typename T>
					static auto& mut(int store_id, int id, T*){
						return internal_store::type_specific_store<T>(store_id,false)[id];
					}
				};
				std::set<int> contains_set; //keys that we can map

				template<typename T>
				const std::unique_ptr<T>& at(int i) const{
					T* dummy{nullptr};
					assert(contains(i));
					return internal_store_shared::at(store_id, i,dummy);
				}

				template<typename T>
				auto &mut(int id){
					T* dummy{nullptr};
					return internal_store_shared::mut(store_id,id,dummy);
				}

				bool contains(int i) const{
					return (contains_set.count(i) > 0);
				}

				int count(int i) const {
					return contains_set.count(i);
				}

				virtual ~internal_store(){
					new_id(store_id);
				}
				
			};

			internal_store store_impl;


			template<StoreType ss,restrict(ss != semantic_switch)>
			explicit StoreMap(StoreMap<ss> &&sm)
				:looping(sm.looping),
				 above(sm.above),
				 store_impl(std::move(sm.store_impl)),
				 valid_store(sm.valid_store){
				sm.valid_store = false;
			}
	
		public:
	
			static std::map<int, StoreMap*>& lost_and_found() {
				return *((std::map<int, StoreMap*>*) &store_lost_and_found_map);
			};

			bool looping = false;
			StoreMap const * const above{nullptr};
			void in_loop() {
				assert(is_store(semantic_switch));
				//yeah, no nested while-loops for now.
				//will be obvious later.
				assert(!looping);
				looping = true;
			}
			void out_of_loop() {
				assert(looping);
				assert(is_store(semantic_switch));
				looping = false;
			}
	
			bool valid_store = true;
			bool contains(int i) const{
				return store_impl.contains(i) ||
					(above && (above->contains(i)));
			}

			StoreMap(){}

			template<StoreType ss,restrict(ss != semantic_switch)>
			explicit StoreMap(const StoreMap<ss> &sm):above((StoreMap const * const) &sm){}

			StoreMap(const StoreMap&) = delete;

#define dbg_store_prnt(y,x) //if (is_store(semantic_switch)) std::cout << y << " this value (" << i << "): " << x << std::endl;
#define dbg_store_prnt2 //if (is_store(semantic_switch)) std::cout << "Getting this value (" << i << "): " << *ret << std::endl;
	
			template<typename T>
			void insert(int i, const T &item) {
				assert(valid_store);
				if (!looping) assert(!contains(i));
				store_impl.template mut<T>(i).reset(mutils::heap_copy(item).release());
				lost_and_found()[i] = this;
				assert(contains(i));
				dbg_store_prnt("Storing",item)
					}

			template<typename T, typename... Args>
			void emplace_ovrt(int i, Args && ... args){
				assert(valid_store);
				store_impl.template mut<T>(i).reset(new T(std::forward<Args>(args)...));
				lost_and_found()[i] = this;
				assert(contains(i));
				dbg_store_prnt("Emplacing",*((T*) store_impl.at<T>(i).get()))
					}

			template<typename T, typename... Args>
			void emplace(int i, Args && ... args){
				assert(valid_store);
				if (!looping) assert(!contains(i));
				emplace_ovrt<T>(i,std::forward<Args>(args)...);
			}

		private:
			auto get_assert(int i) const {
				if (!contains(i)) {
					std::cerr << "trying to find something of id " << i
							  << " in a " << (is_store(semantic_switch) ? "store" : "cache" ) <<std::endl;
					std::cerr << "we have that here : "	<< lost_and_found()[i] <<std::endl;
					throw StoreMiss{};
				}	
			}
			
		public:

			
			template<typename T>
			const T& get(int i) const{
				get_assert(i);
				if (!store_impl.contains(i)) {
					return above->template get<T>(i);
				}
				T* ret = (store_impl.template at<T>(i).get());
				assert(ret);
				dbg_store_prnt2;
				return *ret;
			}
			
			virtual ~StoreMap() {
				valid_store = false;
			}

			friend struct Transaction;
		};

		using StrongStore = StoreMap<StoreType::StrongStore>;
		using StrongCache = StoreMap<StoreType::StrongCache>;
		using CausalStore = StoreMap<StoreType::CausalStore>;
		using CausalCache = StoreMap<StoreType::CausalCache>;

		template<typename T>
		struct is_Cache :
		std::integral_constant<
			bool,
			std::is_same<StrongCache, std::decay_t<T> >::value ||
			std::is_same<CausalCache, std::decay_t<T> >::value >::type
		{};

	} }
