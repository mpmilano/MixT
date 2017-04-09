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
			GStoreContext() = default;
		protected:
			~GStoreContext() = default;
    };
    template<typename l>
    struct StoreContext : public GStoreContext {
      virtual _DataStore<l,l::might_track::value>& store() = 0;
			StoreContext(const StoreContext&) = delete;
			StoreContext() = default;
			virtual ~StoreContext() = default;
    };

		template<typename label>
		struct PhaseContext {
			std::unique_ptr<StoreContext<label> > s_ctx;
			template<typename l>
			using DataStore = _DataStore<l,l::might_track::value>;
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

		};
    
  }}
