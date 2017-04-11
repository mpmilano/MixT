#pragma once

#include "GDataStore.hpp"
#include "Basics.hpp"

namespace myria {

template <typename l> class DataStore : public virtual GDataStore {
public:
  // we'll delete the TransactionContext
  // when the transaction is over.  Do any cleanup you need to do then.
  // the parameters to this function should just be passed directly to
  // TransactionContext's constructor.
  virtual std::unique_ptr<mtl::StoreContext<l>> begin_transaction(
#ifndef NDEBUG
      const std::string &why
#endif
      ) = 0;
  _DataStore() : GDataStore(l::description) {}
  virtual ~_DataStore() = default;
};

  struct TrackableDataStore_super : public virtual GDataStore {
    virtual std::unique_ptr<LabelFreeHandle<Tombstone> > new_tomb (GPhaseContext *ctx, Name, const Tombstone&) = 0;
    virtual bool exists (GPhaseContext *_ctx, Name) = 0;
    virtual std::unique_ptr<LabelFreeHandle<Clock> > existing_clock (GPhaseContext *_ctx, Name) = 0;
    virtual std::unique_ptr<LabelFreeHandle<Tombstone> > existing_tombstone (GPhaseContext *_ctx, Name);
    virtual ~TrackableDataStore_super() == default;
  };

  struct WeakTrackableDataStore : public virtual TrackableDataStore_super{
    virtual const std::array<int, NUM_CAUSAL_GROUPS> &local_time() const = 0;
    virtual ~WeakTrackableDataStore() = default;
  };

  struct StrongTrackableDataStore : public virtual TrackableDataStore_super{
    virtual ~StrongTrackableDataStore() = default;
  };

  template<typename DS> struct TrackableDataStore_common : virtual public TrackableDataStore_super{
    using label = typename DS::label;
    std::unique_ptr<LabelFreeHandle<Tombstone> > new_tomb (GPhaseContext *_ctx, Name n, const Tombstone& val) {
      DS *ds = dynamic_cast<DS*>(this);
      assert(ds);
      typename DS::StoreContext *ctx = ((mtl::PhaseContext<label> *)_ctx)->store_context(*ds whendebug(, "tracker wants a new tombstone"));
      auto ret = ds->newObject(ctx,n,val);
      //erase operation support, if any
      Handle<label,Tombstone> h = ret;
      return std::unique_ptr<LabelFreeHandle<Tombstone> >{new DECT(h){h}};
    }
    //bool exists (GPhaseContext *_ctx, Name);
    bool exists (GPhaseContext *_ctx, Name n){
      DS *ds = dynamic_cast<DS*>(this);
      assert(ds);
      typename DS::StoreContext *ctx = ((mtl::PhaseContext<label> *)_ctx)->store_context(*ds whendebug(, "tracker wants to see if something exists"));
      return ds->exists(ctx,n);
    }
    
    std::unique_ptr<LabelFreeHandle<Clock> > existing_clock (GPhaseContext *_ctx, Name n){
      DS *ds = dynamic_cast<DS*>(this);
      assert(ds);
      typename DS::StoreContext *ctx = ((mtl::PhaseContext<label> *)_ctx)->store_context(*ds whendebug(, "tracker wants an existing clock"));
      auto ret = ds->template existingObject<Clock>(ctx,n);
      //erase operation support, if any
      Handle<label,Clock> h = ret;
      return std::unique_ptr<LabelFreeHandle<Clock> >{new DECT(h){h}};
    }
    std::unique_ptr<LabelFreeHandle<Tombstone> > existing_tombstone (GPhaseContext *_ctx, Name){
      DS *ds = dynamic_cast<DS*>(this);
      assert(ds);
      typename DS::StoreContext *ctx = ((mtl::PhaseContext<label> *)_ctx)->store_context(*ds whendebug(, "tracker wants an existing tombstone"));
      auto ret = ds->template existingObject<Tombstone>(ctx,n);
      //erase operation support, if any
      Handle<label,Tombstone> h = ret;
      return std::unique_ptr<LabelFreeHandle<Tombstone> >{new DECT(h){h}};
    }
    virtual ~TrackableDataStore_common() = default;
  };

//to support tracking, your datastore needs a few extra
//methods.  We ensure they are available here.
  template<typename DS, bool weak> struct _TrackableDataStore;
  template<typename DS> struct _TrackableDataStore<DS,true> : public WeakTrackableDataStore, public TrackableDataStore_common<DS>, public Datastore<typename DS::label> {

  };
  
  template<typename DS> struct _TrackableDataStore<DS,false> : public StrongTrackableDataStore, public TrackableDataStore_common<DS>, public Datastore<typename DS::label> {

  };

  template<typename DS> using TrackableDataStore = _TrackableDataStore<DS,DS::label::might_track>;

}
