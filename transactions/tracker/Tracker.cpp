#include "DataStore.hpp"
#include <cstdlib>
#include <time.h>
#include "GDataStore.hpp"
#include "compile-time-lambda.hpp"
#include "utils.hpp"
#include <functional>
#include <array>
#include <map>
#include <list>
#include <set>
#include <chrono>
#include <thread>
#include <unistd.h>

#include "CooperativeCache.hpp"
#include "Tracker_common.hpp"
#include "CompactSet.hpp"
#include "Tracker_support_structs.hpp"
#include "Ends.hpp"
#include "Transaction.hpp"
#include "TransactionBasics.hpp"
#include "SafeSet.hpp"
#include "Ostreams.hpp"
#include "Tracker_private_declarations.hpp"
namespace {
	constexpr unsigned long long bigprime_lin =
#include "big_prime"
		;
}
#include <cstdlib>

namespace myria { namespace tracker { 

		using namespace std;
		using namespace chrono;
		using namespace TDS;
		using namespace mutils;
		using namespace mtl;
		using namespace tracker;

		Bundle::Bundle(std::future<CooperativeCache::obj_bundle> f)
			:f(new decltype(f)(std::move(f))){}

		Bundle::Bundle(){}

		CooperativeCache::obj_bundle& Bundle::get() {
			if (f->valid()){
				p = heap_copy(f->get());
			}
			assert(!f->valid());
			return *p;
		}
		
		Bundle::~Bundle(){
			//TODO: there's a memory error here somewhere.  Just leak for now.
			/*
			  if (f->valid()){
			  assert(f.use_count() > 0);
			  destroyed_bundles()->emplace(std::async(std::launch::async,[f = this->f]() -> bool {
			  while (f->wait_for(1ms) == future_status::timeout) {}
			  return true;
			  }));
			  }//*/
		}
		

		const DataStore<Level::strong>& Tracker::get_StrongStore() const {
			assert(i->registeredStrong);
			return *i->registeredStrong;
		}
		
		const DataStore<Level::causal>& Tracker::get_CausalStore() const {
			assert(i->registeredCausal);
			return *i->registeredCausal;
		}

		DataStore<Level::strong>& Tracker::get_StrongStore() {
			assert(i->registeredStrong);
			return *i->registeredStrong;
		}
		
		DataStore<Level::causal>& Tracker::get_CausalStore() {
			assert(i->registeredCausal);
			return *i->registeredCausal;
		}

		bool Tracker::strongRegistered() const {
			return i->registeredStrong;
		}

		bool Tracker::causalRegistered() const {
			return i->registeredCausal;
		}


		void TrackingContext::commitContext(){
				i->_commitContext();
				i->_finalize();
		}
		void TrackingContext::abortContext(){
				i->_finalize();
		}

                TrackingContext::TrackingContext(std::unique_ptr<mutils::abs_StructBuilder> &logger, Tracker &trk, bool cod)
                        :i(new TrackingContext::Internals{*trk.i,cod}),trk(trk),logger(logger){}

		TrackingContext::~TrackingContext(){
			if (i) delete i;
		}

                std::unique_ptr<TrackingContext> Tracker::generateContext(std::unique_ptr<mutils::abs_StructBuilder> &logger, bool commitOnDelete){
                        return std::unique_ptr<TrackingContext>{(new TrackingContext{logger, *this,commitOnDelete})};
		}

	}
	namespace mtl{
		void TransactionContext::commitContext(){
			trackingContext->commitContext();
		}
		void TransactionContext::abortContext(){
			trackingContext->abortContext();
		}
	}
	namespace tracker{

		namespace {
			void remove_pending(TrackingContext::Internals& ctx, Tracker::Internals &i, const Name &name){
				ctx.pending_nonces_add.remove_if([&](auto &e){return e.first == name;});
				i.pending_nonces.erase(name);
			}

                        bool tracking_candidate(Tracker &t, Name name, const Tracker::Clock &version){
                                t.updateClock();
                                if (!ends::is_same(version,{{-1,-1,-1,-1}}) && ends::prec(version,t.i->global_min)) {
                                        t.i->tracking.erase(name);
                                        return false;
                                }
                                else return t.i->exceptions.count(name) == 0;
                        }
		}

		void Tracker::assert_nonempty_tracking() const {
			assert (!(i->tracking.empty()));
		}

		const CooperativeCache& Tracker::getcache() const {
			return i->cache;
		}
		
		Tracker::Tracker(int cache_port, CacheBehaviors beh):
			i{new Internals{beh}},cache_port(cache_port){
				assert(cache_port > 0 && "error: must specify non-zero cache port for first tracker call");
				i->cache.listen_on(cache_port);
				//std::cout << "tracker built" << std::endl;
			}

		Tracker::~Tracker(){
			delete i;
		}

		Name Tracker::Tombstone::name() const {
			return nonce;
		}

		void Tracker::registerStore(DataStore<Level::strong>& ds, std::unique_ptr<TrackerDSStrong> wds){
			assert(i->registeredStrong == nullptr);
			assert(i->strongDS.get() == nullptr);
			i->registeredStrong = &ds;
			i->strongDS = std::move(wds);
		}

		void Tracker::registerStore(DataStore<Level::causal>& ds, std::unique_ptr<TrackerDSCausal> wds){
			assert(i->registeredCausal == nullptr);
			assert(i->causalDS.get() == nullptr);
			i->registeredCausal = &ds;
			i->causalDS = std::move(wds);
		}

		void Tracker::exemptItem(Name name){
			i->exceptions.insert(name);
		}

		bool Tracker::registered(const GDataStore& gds) const{
			if (auto *t1 = dynamic_cast<const DataStore<Level::strong>* >(&gds)){
				return t1 == i->registeredStrong;
			}
			else if (auto *t2 = dynamic_cast<const DataStore<Level::causal>*  >(&gds)){
				return t2 == i->registeredCausal;
			}
			else assert(false && "there's a third kind of GDataStore?");
		}

		namespace{


			//constexpr int bigprime_causal = 2751103;
	
			bool is_metaname(long int base, Name name){
				return (name > 0) && ((name % base) == 0);
			}

			Name make_metaname(long int base, Name name){
				assert([=](){Name sanity = numeric_limits<int>::max() * base; return sanity > 0;}());
				assert(name <= numeric_limits<int>::max());
				assert(name > 0);
				assert(!is_metaname(base,name));
				Name cand = name * base;
				assert(cand > 0);
				assert(is_metaname(base,cand));
				return cand;
			}

			bool is_lin_metadata(Name name){
				return is_metaname(bigprime_lin,name);
			}

			Name make_lin_metaname(Name name){
				return make_metaname(bigprime_lin,name);
			}
		}

		namespace {
	
			int get_ip() {
				static int ip_addr{[](){
						std::string static_addr {MY_IP};
						if (static_addr.length() == 0) static_addr = "127.0.0.1";
						return mutils::decode_ip(static_addr);
					}()};
				return ip_addr;
			}

			void write_causal_tombstone(tracker::Tracker& trk, mtl::TransactionContext& ctx, Tracker::Nonce nonce, Tracker::Internals &i){
				using namespace TDS;
				const Tracker::Tombstone t {nonce,get_ip(),trk.cache_port};
				assert(i.cache.contains(nonce));
                                if (get<TDS::exists>(*i.causalDS)(*i.registeredCausal,t.name())) throw mtl::CannotProceedError{"Tomb name collission"};
                                else get<TDS::newTomb>(*i.causalDS)(trk, ctx,*i.registeredCausal,t.name(), t);
			}
		}

		
		void Tracker::onWrite(mtl::TransactionContext& ctx, DataStore<Level::strong>& ds_real, Name name, Tombstone*){}
		void Tracker::onWrite(mtl::TransactionContext& ctx, DataStore<Level::strong>& ds_real, Name name, Clock*){}
		void Tracker::onWrite(mtl::TransactionContext& ctx, DataStore<Level::strong>& ds_real, Name name, void*){
			const auto write_lin_metadata= [this](mtl::TransactionContext& ctx, DataStore<Level::strong> &ds_real,
													  Name name, Tracker::Nonce nonce, Tracker::Internals& t){
				auto &sctx = *ctx.template get_store_context<Level::strong>(ds_real," trying to write lin metadata");
				auto meta_name = make_lin_metaname(name);
				if (get<TDS::exists>(*t.strongDS)(*t.registeredStrong,meta_name)){
                                        get<TDS::existingTomb>(*t.strongDS)(ctx.trackingContext->logger,*t.registeredStrong,meta_name)->put(&sctx,Tracker::Tombstone{nonce,get_ip(),cache_port});
				}
				else {
					get<TDS::newTomb>(*t.strongDS)(*this,ctx,*t.registeredStrong,
												   meta_name,
												   Tracker::Tombstone{nonce,get_ip(),cache_port});
				}
				for (auto &p: i->tracking){
					assert(p.second.second.data());
				}
				i->cache.insert(nonce,i->tracking);
				assert(i->cache.contains(nonce));
			};

			assert(&ds_real == i->registeredStrong);
                        auto tracking_copy = i->tracking;
                        for (auto &pair : tracking_copy){
                            //this will have the side effect of updating the clock,
                            //and removing these items from the tracking set if the clock
                            //is sufficiently recent.
                            tracking_candidate(*this,pair.first,pair.second.first);
                        }
			if (!is_lin_metadata(name) && !i->tracking.empty()){

				auto subroutine = [&](){
					auto nonce = long_rand();
					write_lin_metadata(ctx,ds_real,name,nonce,*i);
					write_causal_tombstone(*this,ctx,nonce,*i);
				};
				bool always_failed = true;
				auto sleep_time = 2ms;
				for (int asdf = 0; asdf < 10; ++asdf){
					try{
						subroutine();
						always_failed = false;
						break;
					}
					catch(const mtl::CannotProceedError&){
						this_thread::sleep_for(sleep_time);
						sleep_time *= 2;
						//assume we picked a bad nonce, try again
					}
				}
				if (always_failed) {
					//it's almost certainly going to fail again, but at this point
					//we are really interested in what the error is.
					subroutine();
				}
				assert(!always_failed);
                                //assert(get<TDS::exists>(*i->strongDS)(ds_real,make_lin_metaname(name)));
			}
		}

/*
  namespace{
  std::ostream & operator<<(std::ostream &os, const Tracker::Clock& c){
  os << "Clock: [";
  for (auto &a : c){
  os << a << ",";
  }
  return os << "]";
  }
  }
*/


		void Tracker::onCreate(DataStore<Level::causal>& ds, Name name,Tombstone*){}
		void Tracker::onCreate(DataStore<Level::causal>& ds, Name name,Clock*){}
		void Tracker::onCreate(DataStore<Level::causal>& ds, Name name,void*){
			//there's nothing to do for a strong datastore right now. maybe there will be later.
		}

		void Tracker::onCreate(DataStore<Level::strong>& , Name, Tombstone*){}
		void Tracker::onCreate(DataStore<Level::strong>& , Name, Clock*){}
		void Tracker::onCreate(DataStore<Level::strong>& , Name, void*){
			//there's nothing to do for a strong datastore right now. maybe there will be later.
		}

		void Tracker::onWrite(DataStore<Level::causal>&, Name name, const Clock &version, Tombstone*){}
		void Tracker::onWrite(DataStore<Level::causal>&, Name name, const Clock &version, Clock*){}
		void Tracker::onWrite(DataStore<Level::causal>&, Name name, const Clock &version, void*){
			//there's nothing to do for a strong datastore right now. maybe there will be later.	
		}


		namespace{

			bool sleep_on(TrackingContext::Internals &ctx, Tracker::Internals& i, const Name &tomb_name, const int how_long = -1){
				bool first_skip = true;
				for (int cntr = 0; (cntr < how_long) || how_long == -1; ++cntr){
					if (get<TDS::exists>(*i.causalDS)(*i.registeredCausal,tomb_name)){
						//if (!first_skip) std::cout << "done waiting" << std::endl;
						remove_pending(ctx,i,tomb_name);
						return true;
					}
					else {
						if (first_skip){
							//std::cout << "waiting for " << tomb_name << " to appear..." << std::endl;
							first_skip = false;
						}
						std::this_thread::sleep_for(10ms);
					}
				}
				return false;
			}
			

			template<typename P>
				std::vector<char> const * const wait_for_available(TrackingContext::Internals &ctx, Tracker::Internals &i, Name name, P& p, const Tracker::Clock &v){
				if (get<TDS::exists>(*i.causalDS)(*i.registeredCausal,p.first)){
					remove_pending(ctx,i,p.first);
					return nullptr;
				}
				else {
					try{
						auto &remote = p.second.get();
						auto ret = CooperativeCache::find(remote,name,v);
						assert(ret->data());
						return ret;
					}
					catch (const std::exception &e){
						//something went wrong with the cooperative caching
						//std::cout << "Cache request failed! Waiting for tombstone" << std::endl;
						//std::cout << "error message: " << e.what() << std::endl;
						
						sleep_on(ctx,i,p.first);
						assert(get<TDS::exists>(*i.causalDS)(*i.registeredCausal,p.first));
						remove_pending(ctx,i,p.first);
						return nullptr;
					}
					catch(...){
						std::cerr << "this is a very bad error" << std::endl;
						assert(false && "whaaat");
					}
				}
			}
		}
		
		void Tracker::afterRead(mtl::StoreContext<Level::strong> &sctx, TrackingContext &tctx, DataStore<Level::strong>& ds, Name name, Tombstone*){}
		void Tracker::afterRead(mtl::StoreContext<Level::strong> &sctx, TrackingContext &tctx, DataStore<Level::strong>& ds, Name name, Clock*){}
		void Tracker::afterRead(mtl::StoreContext<Level::strong> &sctx, TrackingContext &tctx, DataStore<Level::strong>& ds, Name name, void*){

			assert(name != 1);
			
			assert(&ds == i->registeredStrong);
			if (!is_lin_metadata(name)){
                                updateClock();
				auto ts = make_lin_metaname(name);
				if (get<TDS::exists>(*i->strongDS)(ds,ts)){
                                        tctx.logger->incrementIntField(
						LogFields::tracker_strong_afterread_tombstone_exists);
					auto tomb_p = get<TDS::existingTomb>(*i->strongDS)(tctx.logger,ds,ts)->get(&sctx,this,&tctx);
					auto &tomb = *tomb_p;
					if (!sleep_on(*tctx.i,*i,tomb.name(),2)){
                                                tctx.logger->incrementIntField(
							LogFields::tracker_strong_afterread_nonce_unavailable);
						//std::cout << "Nonce isn't immediately available, adding to pending_nonces" << std::endl;
						tctx.i->pending_nonces_add.emplace_back
							(tomb.name(), Bundle{i->cache.get(tomb)});
					}
				}
			}
		}

#define for_each_pending_nonce(ctx,i,f...)			\
		{for (auto &p : i->pending_nonces){			\
				f/*(p)*/;									\
		}													\
		for (auto &p : ctx->pending_nonces_add){			\
			f/*(p)*/;										\
		}}													\


		bool Tracker::waitForRead(TrackingContext &ctx, DataStore<Level::causal>&, Name name, const Clock& version, Tombstone*){
			return true;
		}
		bool Tracker::waitForRead(TrackingContext &ctx, DataStore<Level::causal>&, Name name, const Clock& version, Clock*){
			return true;
		}
//for when merging locally is too hard or expensive.  Returns "true" when candidate version is fine to return, "false" otherwise
		bool Tracker::waitForRead(TrackingContext &ctx, DataStore<Level::causal>&, Name name, const Clock& version, void*){
			//TODO: distinctly not thread-safe
			//if the user called onRead manually and did a merge,
			//we don't want to wait here.This has been ongoing for a couple weeks
			//since this is always called directly after
			//the user had the chance to use onRead,
			//we can use this trivial state tracking mechanism
			//std::cout << "break 1!" << std::endl;
			if (i->last_onRead_name && *i->last_onRead_name == name){
				//std::cout << "break 1: we used onRead, so we're skipping this" << std::endl;
				i->last_onRead_name.reset(nullptr);
				return true;
			}

                        if (tracking_candidate(*this,name,version)) {
				//std::cout << "break 1: this is a tracking candidate" << std::endl;
				//need to pause here and wait for nonce availability
				//for each nonce in the list

				{for (auto &p : i->pending_nonces){
						//std::cout << "break 1: checking wait_for_available" << std::endl;
						if (wait_for_available(*ctx.i,*i,name,p,version)){
							//std::cout << "break 1: wait_for_available in if-condition" << std::endl;
							//I know we've gotten a cached version of the object,
							//but we can't merge it, so we're gonna have to
							//hang out until we've caught up to the relevant tombstone
							//std::cout << "Cache request succeeded!  But we don't know how to merge.."
							//		  << std::endl;
							sleep_on(*ctx.i,*i,p.first);
							assert(get<TDS::exists>(*i->causalDS)(*i->registeredCausal,p.first));
							
						}
					}
					for (auto &p : ctx.i->pending_nonces_add){
						//std::cout << "break 1: checking wait_for_available" << std::endl;
						if (wait_for_available(*ctx.i,*i,name,p,version)){
							//std::cout << "break 1: wait_for_available in if-condition" << std::endl;
							//I know we've gotten a cached version of the object,
							//but we can't merge it, so we're gonna have to
							//hang out until we've caught up to the relevant tombstone
							//std::cout << "Cache request succeeded!  But we don't know how to merge.."
							//		  << std::endl;
							sleep_on(*ctx.i,*i,p.first);
							assert(get<TDS::exists>(*i->causalDS)(*i->registeredCausal,p.first));
						}
					}}											
				return false;
			}
			return true;
		}

		void Tracker::afterRead(TrackingContext &tctx, DataStore<Level::causal>&, Name name, const Clock& version, const std::vector<char> &data, Tombstone*){}
		void Tracker::afterRead(TrackingContext &tctx, DataStore<Level::causal>&, Name name, const Clock& version, const std::vector<char> &data, Clock*){}
		void Tracker::afterRead(TrackingContext &tctx, DataStore<Level::causal>&, Name name, const Clock& version, const std::vector<char> &data, void*){
                        if (tracking_candidate(*this,name,version)){
				tctx.logger->incrementIntField(LogFields::tracker_causal_afterread_candidate);
				//need to overwrite, not occlude, the previous element.
				//C++'s map semantics are really stupid.
                                tctx.i->tracking_erase.push_back(name);
				assert(data.data());
				assert(data.size() > 0);
				//std::cout << data << std::endl;
				for (auto &i : version) assert(i != -1);
				if (!ends::prec(version,i->global_min)){
					tctx.i->tracking_add.emplace_back(name,std::make_pair(version,data));
					//std::cout << tctx.i->tracking_add.back().second.second << std::endl;
					assert(tctx.i->tracking_add.back().second.second.data());
				}
			}
		}

//for when merging is the order of the day
		void Tracker::onRead(TrackingContext &ctx, DataStore<Level::causal>&, Name name, const Clock &version,
							 const std::function<void (char const *)> &construct_and_merge){
			i->last_onRead_name = heap_copy(name);
                        if (tracking_candidate(*this,name,version)){
				//need to pause here and wait for nonce availability
				//for each nonce in the list
				for_each_pending_nonce(
					ctx.i,i,
					if (auto* remote_vers = wait_for_available(*ctx.i,*i,name,p,version)){
						//build + merge real object
						assert(remote_vers->data());
						construct_and_merge(remote_vers->data());
						return;
						
							/**
							   There are many pending nonces; any of them could be in our tracking set
							   due to a dependency on this object. Cycle through them until we find one
							   that is (or fail to find any that are); this one will tell us a place to get
							   the object from the cooperative cache. Grab the object from the cache there.
							   If that fails, then too bad; handling that comes later.
							 */
						}
					else assert(get<TDS::exists>(*i->causalDS)(*i->registeredCausal,p.first));
					);
			}
			return;
		}
		
		void Tracker::onRead(TrackingContext&,DataStore<Level::strong>&, Name, const Clock &,
							 const std::function<void (char const *)> &construct_nd_merge){}
	}}
