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
	constexpr int bigprime_lin =
#include "big_prime"
		;
}
#include <cstdlib>

namespace myria { namespace tracker { 

		using namespace std;
		using namespace chrono;
		using namespace TDS;

		struct Tracker::Internals{
			Internals(const Internals&) = delete;
			DataStore<Level::strong> *registeredStrong {nullptr};
			std::unique_ptr<TrackerDSStrong > strongDS;

			DataStore<Level::causal> *registeredCausal {nullptr};
			std::unique_ptr<TrackerDSCausal > causalDS;

			Clock global_min;

			std::map<int, pair<Clock, vector<char> > > tracking;
			std::set<int> exceptions;
			CooperativeCache cache;
		};

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
	
			bool is_metaname(int base, Name name){
				return (name % base) == 0;
			}

			int make_metaname(int base, Name name){
				assert(!is_metaname(base,name));
				int cand;
                if (name > (5 * base))
					cand = (name / base) * base;
				else cand = name * base;
				assert(is_metaname(base,cand));
				return cand;
			}

			bool is_lin_metadata(Name name){
				return is_metaname(bigprime_lin,name);
			}

			int make_lin_metaname(Name name){
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
						int ret = 0;
						std::string static_addr {MY_IP};
						if (static_addr.length() == 0) static_addr = "127.0.0.1";
						char* iparr = (char*) &ret;
						std::stringstream s(static_addr);
						char ch; //to temporarily store the '.'
						s >> iparr[0] >> ch >> iparr[1] >> ch >> iparr[2] >> ch >> iparr[3];
						return ret;
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
						auto nonce = mutils::long_rand();
						write_lin_metadata(name,nonce,*i);
						write_causal_tombstone(nonce,*i);
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

		void Tracker::onRead(DataStore<Level::strong>& ds, Name name){
			using update_clock_t = void (*)(Tracker::Internals &t);
			static update_clock_t update_clock = [](Tracker::Internals &t){
				auto newc = get<TDS::existingClock>(*t.strongDS)(*t.registeredStrong, bigprime_lin)->get();
				assert(ends::prec(t.global_min,newc));
				t.global_min = newc;
				list<int> to_remove;
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
					while (true){
						if (get<TDS::exists>(*i->causalDS)(*i->registeredCausal,tomb.name())){
							return;
						}
						else {
							std::cout << "waiting for " << tomb.name() << " to appear..." << std::endl;
							std::this_thread::sleep_for(10ms);
							auto bundle = i->cache.get(tomb, cache_port);
							//bundle contains a set of (object-name, version-timestamp, object-as-bytes)
							//we shouldn't actually do the bundle fetch here.
							//what we should do is start the bundle fetch here asynchronously,
							//and block any subsequent causal operations until either the bundle
							//fetch completes or the tombstone shows up at causal storage.
						}
					}
				}
			}
		}

		void Tracker::onCreate(DataStore<Level::causal>& ds, Name name){
			//there's nothing to do for a strong datastore right now. maybe there will be later.
		}

		void Tracker::onCreate(DataStore<Level::strong>& , Name ){
			//there's nothing to do for a strong datastore right now. maybe there will be later.
		}

		void Tracker::onWrite(DataStore<Level::causal>&, Name name, const Clock &version){
			//there's nothing to do for a strong datastore right now. maybe there will be later.	
		}

		void Tracker::onRead(DataStore<Level::causal>&, Name name, const Clock &version, std::vector<char> bytes){
			if (ends::prec(version,i->global_min)) {
				i->tracking.erase(name);
			}
			else if (i->exceptions.count(name) == 0) {
				//need to overwrite, not occlude, the previous element.
				//C++'s map semantics are really stupid. 
				i->tracking.erase(name);
				i->tracking.emplace(name,make_pair(version,bytes));
			}
		}
	}}
