#pragma once

namespace myria {
  struct GDataStore;
  template<Level l>
  class DataStore;
  
  namespace mtl {
    struct GStoreContext{
      virtual GDataStore& store() = 0;
      virtual bool store_commit() = 0;
      virtual void store_abort() = 0;
      virtual ~GStoreContext() = default;
    };
    template<typename l>
    struct StoreContext : public GStoreContext {
      virtual DataStore<l>& store() = 0;
      virtual ~StoreContext() = default;
    };
    
  }}
