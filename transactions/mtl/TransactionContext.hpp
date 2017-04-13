#pragma once
#include "../tracker/TrackingContext.hpp"
#include "top.hpp"
#include "macro_utils.hpp"
#include <memory>

namespace myria {
struct GDataStore;
template <typename>
class DataStore;

namespace tracker {
class Tracker;
}

namespace mtl {
struct GStoreContext
{
  virtual GDataStore& store() = 0;
  virtual bool store_commit() = 0;
  virtual bool store_abort() = 0;
  GStoreContext() = default;

protected:
  ~GStoreContext() = default;
};
template <typename l>
struct StoreContext : public GStoreContext
{
  virtual DataStore<l>& store() = 0;
  StoreContext(const StoreContext&) = delete;
  StoreContext() = default;
  virtual ~StoreContext() = default;
};

struct GPhaseContext
{
  GPhaseContext() = default;
  virtual GStoreContext* store_context() = 0;
  virtual ~GPhaseContext() = default;
  bool store_abort()
  {
    auto *s_ctx = store_context();
    if (s_ctx)
      return s_ctx->store_abort();
    else
      return true;
  }
};

struct TrackedPhaseContext : public GPhaseContext
{
  tracker::TrackingContext trk_ctx;
  TrackedPhaseContext(tracker::Tracker &trk);
  virtual GStoreContext* store_context() = 0;
  virtual ~TrackedPhaseContext() = default;
  bool store_abort()
  {
    auto *s_ctx = store_context();
    if (s_ctx)
      return s_ctx->store_abort();
    else
      return true;
  }
};
  template <typename label, bool tracked>
  struct _PhaseContext;
  template <typename l>
  struct _PhaseContext<Label<l>,true> : public TrackedPhaseContext
  {
    using label = Label<l>;
    std::unique_ptr<StoreContext<label>> s_ctx;
    StoreContext<label>& store_context(DataStore<label>& ds whendebug(, const std::string& why))
    {
      if (!s_ctx) {
	s_ctx = ds.begin_transaction(whendebug(why));
      }
      return *s_ctx;
    }
    _PhaseContext(tracker::Tracker& trk);
    StoreContext<label>* store_context(){ return s_ctx.get(); }
  };
  template <typename l>
  struct _PhaseContext<Label<l>,false> : public GPhaseContext
  {
    using label = Label<l>;
    std::unique_ptr<StoreContext<label>> s_ctx;
    StoreContext<label>& store_context(DataStore<label>& ds whendebug(, const std::string& why))
    {
      if (!s_ctx) {
	s_ctx = ds.begin_transaction(whendebug(why));
      }
      return *s_ctx;
    }
    StoreContext<label>* store_context(){ return s_ctx.get(); }
    _PhaseContext() = default;
    _PhaseContext(const tracker::Tracker&):_PhaseContext(){}
  };
  template<typename l> using PhaseContext = _PhaseContext<l,l::run_remotely::value>;
}
}
