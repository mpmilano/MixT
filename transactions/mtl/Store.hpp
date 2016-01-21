#pragma once
#include <list>
#include "utils.hpp"
#include "HeteroMap.hpp"

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
		
		struct StoreMap_base {
		private:
			StoreMap_base const * const above{nullptr};
			HeteroMap<int> store_impl;
	
			bool valid_store = true;
			
		protected:

			explicit StoreMap_base(const StoreMap_base& ss, bool ThisIsNotACopyConstructor):above(ss){}

			StoreMap_base(){}
		public:
			StoreMap_base(const StoreMap_base&) = delete;
			
			bool contains(int i) const{
				return store_impl.contains(i) ||
					(above && (above->contains(i)));
			}

			template<typename T>
			void insert(int i, const T &item) {
				assert(valid_store);
				assert(!contains(i));
				store_impl.template mut<T>(i).reset(mutils::heap_copy(item).release());
				assert(contains(i));
			}

			template<typename T, typename... Args>
			void emplace_ovrt(int i, Args && ... args){
				assert(valid_store);
				store_impl.template mut<T>(i).reset(new T(std::forward<Args>(args)...));
				assert(contains(i));
			}

			template<typename T, typename... Args>
			void emplace(int i, Args && ... args){
				assert(valid_store);
				assert(!contains(i));
				emplace_ovrt<T>(i,std::forward<Args>(args)...);
			}
			
			template<typename T>
			const T& get(int i) const{
				//get_assert
				if (!contains(i)) {
					std::cerr << "trying to find something of id " << i
							  << " in a " << (is_store(semantic_switch) ? "store" : "cache" ) <<std::endl;
					std::cerr << "we have that here : "	<< lost_and_found()[i] <<std::endl;
					throw StoreMiss{};
				}

				//actual method
				if (!store_impl.contains(i)) {
					return above->template get<T>(i);
				}
				T* ret = (store_impl.template at<T>(i).get());
				assert(ret);
				return *ret;
			}
		
			virtual ~StoreMap() {
				valid_store = false;
			}

		};

		template<StoreType st>
		struct StoreMap : public StoreMap_base{
			template<StoreType ss,restrict(ss != semantic_switch)>
			explicit StoreMap(const StoreMap<ss> &sm):StoreMap_base(sm,true){}
			StoreMap(){}
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
