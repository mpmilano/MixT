#pragma once
#include "TrackingContext.hpp"
#include "Basics.hpp"
#include "restrict.hpp"

namespace myria {
	
	struct GDataStore;
	template<Level l>
	class DataStore;

	namespace mtl {

		template<typename> struct Transaction;

		template<Level l>
		struct StoreContext{
			virtual DataStore<l>& store() = 0;
			virtual bool store_commit() = 0;
		};

		namespace context{
			
			enum class t{
				unknown, read, write, operation, data, validity
					};
		}

		struct TransactionContext {
			context::t execution_context = context::t::unknown;
			void const * const parameter; //yay safety?
			std::unique_ptr<tracker::TrackingContext> trackingContext;
			std::unique_ptr<StoreContext<Level::strong> > strongContext{nullptr};
			std::unique_ptr<StoreContext<Level::causal> > causalContext{nullptr};
			bool commit_on_delete = false;
			
		private:
			//these two are implemented in Tracker.cpp
			void commitContext ();
			void abortContext ();
			
			bool committed = false;

		public:
			template<typename T>
			TransactionContext(T const * const param, decltype(trackingContext) trackingContext)
				:parameter(param),trackingContext(std::move(trackingContext))
				{}

			TransactionContext(const TransactionContext&) = delete;

			template<typename T>
			TransactionContext(T const * const param, decltype(strongContext) sc, decltype(trackingContext) trackingContext)
				:parameter(param),trackingContext(std::move(trackingContext)),strongContext(std::move(sc))
				{}

			template<typename T>
			TransactionContext(T const * const param, decltype(causalContext) cc, decltype(trackingContext) trackingContext)
				:parameter(param),trackingContext(std::move(trackingContext)),causalContext(std::move(cc))
				{}
			
			bool full_commit();

			template<Level lev, typename Store>
			auto& get_store_context(Store& str){
				choose_strong<lev> choice{nullptr}; //true_type when lev == strong
				auto& store_ctx = get_store_context(choice);
				if (!store_ctx){
					assert(!str.in_transaction());
					store_ctx.reset(str.begin_transaction().release());
				}
				return store_ctx;
			}

			auto& get_store_context(std::true_type*){
				return strongContext;
			}			

			auto& get_store_context(std::false_type*){
				return causalContext;
			}

			virtual ~TransactionContext(){
				if ((!committed) && commit_on_delete) full_commit();
				else if (!committed) abortContext();
			}

			template<typename> friend struct Transaction;
		};
	} }
