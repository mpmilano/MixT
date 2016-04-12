#pragma once
#include <functional>
#include "TrackingContext.hpp"
#include "Basics.hpp"
#include "restrict.hpp"
#include "ObjectBuilder.hpp"

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
			virtual void store_abort() = 0;
			virtual ~StoreContext(){}
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
			std::unique_ptr<VMObjectLog> logger;
			bool commit_on_delete = false;
			
		private:
			//these two are implemented in Tracker.cpp
			void commitContext ();
			void abortContext ();
			
			bool committed = false;

		public:
			TransactionContext(void const * const param, decltype(trackingContext) trackingContext)
				:parameter(param),trackingContext(std::move(trackingContext))
				{}

			TransactionContext(const TransactionContext&) = delete;

			TransactionContext(void const * const param, decltype(strongContext) sc, decltype(trackingContext) trackingContext)
				:parameter(param),trackingContext(std::move(trackingContext)),strongContext(std::move(sc))
				{}

			TransactionContext(void const * const param, decltype(causalContext) cc, decltype(trackingContext) trackingContext)
				:parameter(param),trackingContext(std::move(trackingContext)),causalContext(std::move(cc))
				{}
			
			bool full_commit();
			void full_abort();

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
				else if (!committed) {
					full_abort();
				}
			}

			template<typename> friend struct Transaction;
		};
	} }
