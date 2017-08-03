#pragma once

#include "DataStore.hpp"

namespace myria{

template <typename DS>
std::unique_ptr<LabelFreeHandle<tracker::Tombstone>>
TrackableDataStore_common<DS>::new_tomb_trk(mtl::GPhaseContext *_ctx, Name n,
                                        const tracker::Tombstone &val) {
  using namespace tracker;
	using label = typename DS::label;
  DS *ds = dynamic_cast<DS *>(this);
  assert(ds);
  typename DS::StoreContext &ctx =
		dynamic_cast<typename DS::StoreContext&>(((mtl::PhaseContext<label> *)_ctx)
									 ->store_context(*ds whendebug(, "tracker wants a new tombstone")));
  auto ret = ds->newObject(&ctx, n, val);
  // erase operation support, if any
  Handle<label, Tombstone> h = ret;
  return std::unique_ptr<LabelFreeHandle<tracker::Tombstone>>{new DECT(h){h}};
}
template <typename DS>
bool TrackableDataStore_common<DS>::exists_trk(mtl::GPhaseContext *_ctx, Name n) {
  using namespace tracker;
	using label = typename DS::label;
  DS *ds = dynamic_cast<DS *>(this);
  assert(ds);
  typename DS::StoreContext &ctx =
		dynamic_cast<typename DS::StoreContext&>(((mtl::PhaseContext<label> *)_ctx)
          ->store_context(
						*ds whendebug(, "tracker wants to see if something exists")));
  return ds->exists(&ctx, n);
}

template <typename DS>
std::unique_ptr<LabelFreeHandle<tracker::Clock>>
TrackableDataStore_common<DS>::existing_clock_trk(mtl::GPhaseContext *_ctx,
                                              Name n) {
  using namespace tracker;
	using label = typename DS::label;
  DS *ds = dynamic_cast<DS *>(this);
  assert(ds);
  typename DS::StoreContext &ctx =
		dynamic_cast<typename DS::StoreContext&>(((mtl::PhaseContext<label> *)_ctx)
		 ->store_context(*ds whendebug(, "tracker wants an existing clock")));
  auto ret = ds->template existingObject<Clock>(&ctx, n);
  // erase operation support, if any
  Handle<label, Clock> h = ret;
  return std::unique_ptr<LabelFreeHandle<Clock>>{new DECT(h){h}};
}
template <typename DS>
std::unique_ptr<LabelFreeHandle<tracker::Tombstone>>
TrackableDataStore_common<DS>::existing_tombstone_trk(mtl::GPhaseContext *_ctx,
                                                  Name n) {
  using namespace tracker;
	using label = typename DS::label;
  DS *ds = dynamic_cast<DS *>(this);
  assert(ds);
  typename DS::StoreContext &ctx =
		dynamic_cast<typename DS::StoreContext&>(((mtl::PhaseContext<label> *)_ctx)
          ->store_context(
						*ds whendebug(, "tracker wants an existing tombstone")));
  auto ret = ds->template existingObject<Tombstone>(&ctx, n);
  // erase operation support, if any
  Handle<label, Tombstone> h = ret;
  return std::unique_ptr<LabelFreeHandle<Tombstone>>{new DECT(h){h}};
}
}
