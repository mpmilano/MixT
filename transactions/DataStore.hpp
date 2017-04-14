#pragma once

#include "GDataStore.hpp"
#include "Basics.hpp"

namespace myria {

template <typename T> struct LabelFreeHandle;

namespace tracker {
struct Tombstone;
using Clock = std::array<int, NUM_CAUSAL_GROUPS>;
using Nonce = long;
}

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
  DataStore() : GDataStore(l::description) {}
  virtual ~DataStore() = default;
};

struct TrackableDataStore_super : public virtual GDataStore {
  virtual std::unique_ptr<LabelFreeHandle<tracker::Tombstone>>
  new_tomb(mtl::GPhaseContext *ctx, Name, const tracker::Tombstone &) = 0;
  virtual bool exists(mtl::GPhaseContext *_ctx, Name) = 0;
  virtual std::unique_ptr<LabelFreeHandle<tracker::Clock>>
  existing_clock(mtl::GPhaseContext *_ctx, Name) = 0;
  virtual std::unique_ptr<LabelFreeHandle<tracker::Tombstone>>
  existing_tombstone(mtl::GPhaseContext *_ctx, Name) = 0;
  virtual ~TrackableDataStore_super() = default;
};

struct WeakTrackableDataStore : public virtual TrackableDataStore_super {
  virtual const std::array<int, NUM_CAUSAL_GROUPS> &local_time() const = 0;
  virtual ~WeakTrackableDataStore() = default;
};

struct StrongTrackableDataStore : public virtual TrackableDataStore_super {
  virtual ~StrongTrackableDataStore() = default;
};

template <typename DS>
struct TrackableDataStore_common : virtual public TrackableDataStore_super {
  std::unique_ptr<LabelFreeHandle<tracker::Tombstone>>
  new_tomb(mtl::GPhaseContext *_ctx, Name n, const tracker::Tombstone &val);

  bool exists(mtl::GPhaseContext *_ctx, Name n);

  std::unique_ptr<LabelFreeHandle<tracker::Clock>>
  existing_clock(mtl::GPhaseContext *_ctx, Name n);
  std::unique_ptr<LabelFreeHandle<tracker::Tombstone>>
  existing_tombstone(mtl::GPhaseContext *_ctx, Name);
  virtual ~TrackableDataStore_common() = default;
};

// to support tracking, your datastore needs a few extra
// methods.  We ensure they are available here.
	template <typename DS, typename label, bool weak> struct _TrackableDataStore;
template <typename DS, typename label>
struct _TrackableDataStore<DS, label, true> : public WeakTrackableDataStore,
                                       public TrackableDataStore_common<DS>,
                                       public DataStore<label> {};

	template <typename DS, typename label>
	struct _TrackableDataStore<DS, label, false> : public StrongTrackableDataStore,
                                        public TrackableDataStore_common<DS>,
                                        public DataStore<label> {};

	template <typename DS, typename label>
	using TrackableDataStore = _TrackableDataStore<DS, label, label::might_track::value>;
}
