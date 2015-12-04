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

		namespace{
			struct Bundle{
			private:
				std::shared_ptr<std::future<CooperativeCache::obj_bundle> > f;
				std::shared_ptr<CooperativeCache::obj_bundle> p;

			public:
				Bundle(std::future<CooperativeCache::obj_bundle> f):f(new decltype(f)(std::move(f))){}
				Bundle(){}
				auto& get(){
					if (f->valid()){
						p = heap_copy(f->get());
					}
					assert(!f->valid());
					return *p;
				}
				virtual ~Bundle(){
					if (f->valid()){
						//toss the future into a wrapper thread and exit
						std::thread{[ft = move(*f)](){}}.detach();
					}
				}
			};
		}

		struct Tracker::Internals{
			Internals(const Internals&) = delete;
			DataStore<Level::strong> *registeredStrong {nullptr};
			std::unique_ptr<TrackerDSStrong > strongDS;

			DataStore<Level::causal> *registeredCausal {nullptr};
			std::unique_ptr<TrackerDSCausal > causalDS;

			Clock global_min;

			std::map<Name, std::pair<Clock, std::vector<char> > > tracking;
			std::map<Name, Bundle> pending_nonces;
			std::set<Name> exceptions;
			CooperativeCache cache;
			std::unique_ptr<Name> last_onRead_name{nullptr};
		};

		namespace {
			void remove_pending(Tracker::Internals &i, const Name &name){
				i.pending_nonces.erase(name);
			}
		}

		void Tracker::assert_nonempty_tracking() const {
			assert (!(i->tracking.empty()));
		}

		namespace{
			static constexpr int cache_port = (int{CACHE_PORT} > 0 ? int{CACHE_PORT} : 9889);
		}
		
		Tracker::Tracker():i{new Internals{}}{
			i->cache.listen_on(cache_port);
		}

		Tracker::~Tracker(){
			delete i;
		}

		Name Tracker::Tombstone::name() const {
			return nonce;
		}

		Tracker& Tracker::global_tracker(){
			static Tracker t;
			return t;
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
	
			bool is_metaname(unsigned long long base, Name name){
				return (name % base) == 0;
			}

			Name make_metaname(unsigned long long base, Name name){
				assert(!is_metaname(base,name));
				Name cand;
                if (name > (5 * base))
					cand = (name / base) * base;
				else cand = name * base;
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
			void write_causal_tombstone(Tracker::Nonce nonce, Tracker::Internals &i){
				using namespace TDS;
				const Tracker::Tombstone t {nonce};
				get<TDS::newTomb>(*i.causalDS)(*i.registeredCausal,t.name(), t);
			}
	
			int get_ip() {
				static int ip_addr{[](){
						std::string static_addr {MY_IP};
						if (static_addr.length() == 0) static_addr = "127.0.0.1";
						return mutils::decode_ip(static_addr);
					}()};
				return ip_addr;
			}
		}

		void Tracker::onWrite(DataStore<Level::strong>& ds_real, Name name){
			using wlm_t = void (*)(Name name, Tracker::Nonce nonce, Tracker::Internals& t);
			static const wlm_t write_lin_metadata= [](Name name, Tracker::Nonce nonce, Tracker::Internals& t){
				auto meta_name = make_lin_metaname(name);
				if (get<TDS::exists>(*t.strongDS)(*t.registeredStrong,meta_name)){
					get<TDS::existingTomb>(*t.strongDS)(*t.registeredStrong,meta_name)->put(Tracker::Tombstone{nonce,get_ip()});
				}
				else get<TDS::newTomb>(*t.strongDS)(*t.registeredStrong,
													meta_name,
													Tracker::Tombstone{nonce,get_ip()});
			};

			assert(&ds_real == i->registeredStrong);
			if (!is_lin_metadata(name) && !i->tracking.empty()){

				bool always_failed = true;
				for (int asdf = 0; asdf < 100; ++asdf){
					try{
						auto nonce = long_rand();
						write_lin_metadata(name,nonce,*i);
						write_causal_tombstone(nonce,*i);
						i->cache.insert(nonce,i->tracking);
						assert(false); //add things to Cooperative Cache
						always_failed = false;
						break;
					}
					catch(const mtl::Transaction::CannotProceedError&){
						//assume we picked a bad nonce, try again
					}
				}
				assert(!always_failed);
				assert(get<TDS::exists>(*i->strongDS)(ds_real,make_lin_metaname(name)));
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


		void Tracker::onCreate(DataStore<Level::causal>& ds, Name name){
			//there's nothing to do for a strong datastore right now. maybe there will be later.
		}

		void Tracker::onCreate(DataStore<Level::strong>& , Name ){
			//there's nothing to do for a strong datastore right now. maybe there will be later.
		}

		void Tracker::onWrite(DataStore<Level::causal>&, Name name, const Clock &version){
			//there's nothing to do for a strong datastore right now. maybe there will be later.	
		}


		namespace{

			bool sleep_on(Tracker::Internals& i, const Name &tomb_name, const int how_long = -1){
				for (int cntr = 0; (cntr < how_long) || how_long == -1; ++cntr){
					if (get<TDS::exists>(*i.causalDS)(*i.registeredCausal,tomb_name)){
						remove_pending(i,tomb_name);
						return true;
					}
					else {
						std::cout << "waiting for " << tomb_name << " to appear..." << std::endl;
						std::this_thread::sleep_for(10ms);
					}
				}
				return false;
			}
			
			bool tracking_candidate(Tracker::Internals &i, Name name, const Tracker::Clock &version){
				if (!ends::is_same(version,{{-1,-1,-1,-1}}) && ends::prec(version,i.global_min)) {
					i.tracking.erase(name);
					return false;
				}
				else return i.exceptions.count(name) == 0;
			}

			template<typename P>
				std::vector<char> const * const check_applicable(Tracker::Internals &i, Name name, P& p, const Tracker::Clock &v){
				if (get<TDS::exists>(*i.causalDS)(*i.registeredCausal,p.first)){
					remove_pending(i,p.first);
					return nullptr;
				}
				else {
					try{
						auto &remote = p.second.get();
						return CooperativeCache::find(remote,name,v);
					}
					catch (...){
						//something went wrong with the cooperative caching
						sleep_on(i,p.first);
						return nullptr;
					}
				}
			}
		}
		
		
		void Tracker::afterRead(DataStore<Level::strong>& ds, Name name){
			using update_clock_t = void (*)(Tracker::Internals &t);
			static update_clock_t update_clock = [](Tracker::Internals &t){
				auto newc = get<TDS::existingClock>(*t.strongDS)(*t.registeredStrong, bigprime_lin)->get();
				assert(ends::prec(t.global_min,newc));
				t.global_min = newc;
				list<Name> to_remove;
				for (auto& p : t.tracking){
					if (ends::prec(p.second.first,newc)) to_remove.push_back(p.first);
				}
				for (auto &e : to_remove){
					t.tracking.erase(e);
				}
			};
			assert(&ds == i->registeredStrong);
			if (!is_lin_metadata(name)){
				//TODO: reduce frequency of this call.
				update_clock(*i);
				auto ts = make_lin_metaname(name);
				if (get<TDS::exists>(*i->strongDS)(ds,ts)){
					auto tomb = get<TDS::existingTomb>(*i->strongDS)(ds,ts)->get();
					while (!sleep_on(*i,tomb.name(),1)){
						i->pending_nonces[tomb.name()] = Bundle{i->cache.get(tomb, cache_port)};
					}
				}
			}
		}



//for when merging locally is too hard or expensive
		void Tracker::waitForRead(DataStore<Level::causal>&, Name name, const Clock& version){
			//TODO: distinctly not thread-safe
			//if the user called onRead manually and did a merge,
			//we don't want to wait here.
			//since this is always called directly after
			//the user had the chance to use onRead,
			//we can use this trivial state tracking mechanism
			if (i->last_onRead_name && *i->last_onRead_name == name){
				i->last_onRead_name.reset(nullptr);
				return;
			}
			if (tracking_candidate(*i,name,version)) {
				//need to pause here and wait for nonce availability
				//for each nonce in the list
				if (!i->pending_nonces.empty()){
					for (auto &p : i->pending_nonces){
						if (check_applicable(*i,name,p,version)){
							//hang out until we've caught up to the relevant tombstone
							sleep_on(*i,p.first);
						}
					}
				}
			}
		}
		
		void Tracker::afterRead(DataStore<Level::causal>&, Name name, const Clock& version, const std::vector<char> &data){
			if (tracking_candidate(*i,name,version)){
				//need to overwrite, not occlude, the previous element.
				//C++'s map semantics are really stupid. 
				i->tracking.erase(name);
				i->tracking.emplace(name,std::make_pair(version,data));
			}
		}

//for when merging is the order of the day
		std::unique_ptr<Tracker::MemoryOwner>
		Tracker::onRead(DataStore<Level::causal>&, Name name, const Clock &version,
						//these functions all better play well together
						const std::function<std::unique_ptr<MemoryOwner> ()> &mem,
						const std::function<void (MemoryOwner&, char const *)> &construct_and_merge){
			i->last_onRead_name = heap_copy(name);
			if (tracking_candidate(*i,name,version)){
				//need to pause here and wait for nonce availability
				//for each nonce in the list
				if (!i->pending_nonces.empty()){
					for (auto &p : i->pending_nonces){
						if (auto* remote_vers = check_applicable(*i,name,p,version)){
							auto mo = mem();
							//build + merge real object
							construct_and_merge(*mo,remote_vers->data());
							return mo;
						}
					}
				}
			}
			return nullptr;
		}
		
		std::unique_ptr<Tracker::MemoryOwner> Tracker::onRead(
			DataStore<Level::strong>&, Name name, const Clock &version,
			const std::function<std::unique_ptr<MemoryOwner> ()> &mem,
			const std::function<void (MemoryOwner&, char const *)> &construct_nd_merge){
			return nullptr;
		}
	}}
