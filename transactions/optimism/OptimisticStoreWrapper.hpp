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
				collect_phase, execution_phase
					};

			using StoreContext = OptimisticContext<Store,Operations...>;

			template<typename T>
			using Handle = 
				DECT(std::declval<Store>().newObject(
							 std::declval<StoreContext>(),
							 std::declval<Name>(),
							 std::declval<T>()));

			template<typename T> using StoreObject = OptimisticObject<T>;

			template<typename CTX2, typename Operation, typename... T>
			auto operation(StoreContext &ctx, OperationIdentifier<Operation> id, T&& ... t){
				using super = DECT(*find_match<DECT(Operations::get_match(id))...>());
				return super::operation(*this,ctx, id, std::forward<T>(t)...);
			}

			std::unique_ptr<StoreContext> begin_transaction(
#ifndef NDEBUG
				const std::string& why
#endif
				){
				return new StoreContext(*this);
			}

			static constexpr int id() {
				return 68448;
			}

		};
		
		
	}

}
