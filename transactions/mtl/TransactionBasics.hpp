#pragma once
#include "TrackingContext.hpp"
#include "Basics.hpp"
#include "restrict.hpp"

namespace myria {
	
	struct GDataStore;
	template<Level l>
	class DataStore;

	namespace mtl {

		struct Transaction;

		template<Level l>
		struct StoreContext{
			virtual DataStore<l>& store() = 0;
			virtual bool store_commit() = 0;
		};

		struct TransactionContext {
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
			TransactionContext(decltype(trackingContext) trackingContext)
				:trackingContext(std::move(trackingContext))
				{}

			TransactionContext(decltype(strongContext) sc, decltype(trackingContext) trackingContext)
				:trackingContext(std::move(trackingContext)),strongContext(std::move(sc))
				{}

			TransactionContext(decltype(causalContext) cc, decltype(trackingContext) trackingContext)
				:trackingContext(std::move(trackingContext)),causalContext(std::move(cc))
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

			friend struct Transaction;
		};
	} }
