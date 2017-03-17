#pragma once

namespace myria {
  struct GDataStore;
  template<Level l, bool>
  class _DataStore;
  
  namespace mtl {
    struct GStoreContext{
      virtual GDataStore& store() = 0;
      virtual bool store_commit() = 0;
      virtual void store_abort() = 0;
      virtual ~GStoreContext() = default;
    };
    template<typename l>
    struct StoreContext : public GStoreContext {
      virtual _DataStore<l,l::requires_causal_tracking>& store() = 0;
      virtual ~StoreContext() = default;
    };
    
  }}
