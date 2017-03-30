#pragma once
#include "17_type_utils.hpp"

namespace myria{

	namespace optimism {

		template<typename Store, typename... Operations>
		struct OptimisticStore;

		template<typename Name, typename Ret, typename... Args>
		struct WrappedOperation {

			template<typename Store, typename... Operations>
			static auto operation(OptimisticStore<Store,Operations...> &store,
													 typename OptimisticStore<Store,Operations...>::StoreContext &ctx,
													 OperationIdentifier<Name> id, Args&&... a){
				using super = OptimisticStore<Store,Operations...>;
				if (ctx.phase == super::phases::collect_phase){
					for (hndl : Store::read_set(ctx,id,store.unwrap_ro(std::forward<Args&&>(a))...)){
						ctx.read_set.emplace_back(new DECT(hndl)(hndl));
					}
					for (hndl : Store::write_set(ctx,id,store.unwrap_ro(std::forward<Args&&>(a))...)){
						ctx.write_set.emplace_back(new DECT(hndl)(hndl));
					}
					return Store::simulate_operation(id, store.deref_ro(ctx, std::forward<Args>(a))...);
				}
			}
		};

		template<typename T> using shared_ptr_alias = std::shared_ptr<T>;
		
		template<typename Store, typename... Operations>
		struct OptimisticStore : public DataStore<typename Store::label> {

			static HeteroMap<GeneralRemoteObject*, shared_ptr_alias>& cache(){
				static HeteroMap<GeneralRemoteObject*, shared_ptr_alias> ret;
				return ret;
			}
			std::shared_mutex cache_lock;
			using cache_read_lock = std::shared_lock<decltype(cache_lock)>;

			using wrapped_context = typename Store::StoreContext;
			using label = typename Store::label;
			using StoreHandle = GenericHandle<label>;

			enum class phases{
				collect_phase
			}

			struct StoreContext : public mtl::StoreContext<label>{
				phases phase;
				OptimisticStore &parent;
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

			template<typename T>
			using Handle = 
				DECT(std::declval<Store>().newObject(
							 std::declval<StoreContext>(),
							 std::declval<Name>(),
							 std::declval<T>()));

			template<typename T> struct OptimisticObject : public RemoteObject<label,T> {
				
				OptimisticStore& _store;
				
				Name name;
				Handle hdnl;
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
			template<typename T> using StoreObject = OptimisticObject<T>;

			template<typename CTX2, typename Operation, typename... T>
			auto operation(StoreContext &ctx, OperationIdentifier<Operation> id, T&& ... t){
				using super = DECT(*find_match<DECT(Operations::get_match(id))...>());
				return super::operation(*this,ctx, id, std::forward<T>(t)...);
			}

			template<typename T>
			StoreObject<T>& unwrap_ro(OptimisticObject<T>& o){
				return dynamic_cast<typename Store::template StoreObject<T>& >(o.hndl.remote_object());
			}
			
			template<typename T>
			T unwrap_ro(T&& o){
				return o;
			}

			template<typename T>
			std::shared_ptr<const T> deref_ro(StoreContext& ctx, OptimisticObject<T>& o){
				return o.get(&ctx);
			}
			
			template<typename T>
			T deref_ro(StoreContext& ctx, T&& o){
				return o;
			}

			
		};
		
		
	}

}
