#pragma once
#include "TrackingContext.hpp"
#include "macro_utils.hpp"
#include <memory>

namespace myria {
  struct GDataStore;
  template<typename, bool>
  class _DataStore;

	namespace tracker{
		class Tracker;
	}
	
  namespace mtl {
    struct GStoreContext{
      virtual GDataStore& store() = 0;
      virtual bool store_commit() = 0;
			virtual bool aborted() const = 0;
			GStoreContext() = default;
		protected:
			~GStoreContext() = default;
    };
    template<typename l>
    struct StoreContext : public GStoreContext {
      virtual _DataStore<l,l::requires_causal_tracking::value>& store() = 0;
			StoreContext(const StoreContext&) = delete;
			StoreContext() = default;
			virtual ~StoreContext() = default;
    };

		struct GTransactionContext {
			tracker::Tracker *trk;
			std::unique_ptr<tracker::TrackingContext> trackingContext;
			auto& tracker(){ return *trk;}
			void commitContext();
			void abortContext();
			GTransactionContext(tracker::Tracker& t);
			GTransactionContext() = default;
		protected:
			~GTransactionContext() = default;
		};

		template<typename label>
		struct SingleTransactionContext : public virtual GTransactionContext{
			std::unique_ptr<StoreContext<label> > s_ctx;
			template<typename l>
			using DataStore = _DataStore<l,l::requires_causal_tracking::value>;
			StoreContext<label>& store_context(DataStore<label>& ds whendebug(, const std::string& why)){
				if(!s_ctx){
					s_ctx = ds.begin_transaction(whendebug(why));
				}
				return *s_ctx;
			}
			bool store_abort(){
				if (s_ctx) return s_ctx->store_abort();
				else return true;
			}
		protected:
			~SingleTransactionContext() = default;
		};

		template<typename... labels> 
		struct TransactionContext : public SingleTransactionContext<labels>...{
			TransactionContext(tracker::Tracker& t):GTransactionContext(t){
				assert(this->trk);
			}
		};
    
  }}
