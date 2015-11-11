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
#include <unistd.h>

#include "Tracker_common.hpp"
#include "CompactSet.hpp"
#include "Tracker_support_structs.hpp"
#include "Ends.hpp"


using namespace std;
using namespace TDS;

struct Tracker::Internals{
	Internals(const Internals&) = delete;
	DataStore<Level::strong> *registeredStrong {nullptr};
	std::unique_ptr<TrackerDSStrong > strongDS;

	DataStore<Level::causal> *registeredCausal {nullptr};
	std::unique_ptr<TrackerDSCausal > causalDS;

	Clock global_min;

	std::map<int, Clock > tracking;
	
};

Tracker::Tracker():i{new Internals{}}{
	timespec ts;
	clock_gettime(CLOCK_REALTIME,&ts);
	srand(ts.tv_nsec);
}

Tracker::~Tracker(){
	delete i;
}

int Tracker::Tombstone::name() const {
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

	constexpr int bigprime_lin =
#include "big_prime"
		;
	//constexpr int bigprime_causal = 2751103;
	
	bool is_metaname(int base, int name){
		return (name % base) == 0;
	}

	int make_metaname(int base, int name){
		assert(!is_metaname(base,name));
		int cand;
                if (name > (5 * base))
			cand = (name / base) * base;
		else cand = name * base;
		assert(is_metaname(base,cand));
		return cand;
	}

	bool is_lin_metadata(int name){
		return is_metaname(bigprime_lin,name);
	}

	int make_lin_metaname(int name){
		return make_metaname(bigprime_lin,name);
	}
}

namespace {
	void write_lin_metadata(int name, Tracker::Nonce nonce, Tracker::Internals& t){
		auto meta_name = make_lin_metaname(name);
		get<TDS::newTomb>(*t.strongDS)(*t.registeredStrong,
									   meta_name,
									   Tracker::Tombstone{nonce});
	}
	void write_causal_tombstone(Tracker::Nonce nonce, Tracker::Internals &i){
		using namespace TDS;
		const Tracker::Tombstone t {nonce};
		get<TDS::newTomb>(*i.causalDS)(*i.registeredCausal,t.name(), t);
	}
}

void Tracker::onWrite(DataStore<Level::strong>& ds_real, int name){
	assert(&ds_real == i->registeredStrong);
	if (!is_lin_metadata(name) && !i->tracking.empty()){
		assert(false);
		auto nonce = rand();
		write_lin_metadata(name,nonce,*i);
		write_causal_tombstone(nonce,*i);
		assert(get<TDS::exists>(*i->strongDS)(ds_real,make_lin_metaname(name)));
	}
}

namespace{
	std::ostream & operator<<(std::ostream &os, const Tracker::Clock& c){
		os << "Clock: [";
		for (auto &a : c){
			os << a << ",";
		}
		return os << "]";
	}
}

void Tracker::onRead(DataStore<Level::strong>& ds, int name){
	using update_clock_t = void (*)(Tracker::Internals &t);
	static update_clock_t update_clock = [](Tracker::Internals &t){
		auto newc = get<TDS::existingClock>(*t.strongDS)(*t.registeredStrong, bigprime_lin)->get();
		assert(ends::prec(t.global_min,newc));
		t.global_min = newc;
		list<int> to_remove;
		for (auto& p : t.tracking){
			if (ends::prec(p.second,newc)) to_remove.push_back(p.first);
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
					assert(false);
					return;
				}
				else sleep(1);
			}
		}
	}
}

void Tracker::onCreate(DataStore<Level::causal>& ds, int name){
    //there's nothing to do for a strong datastore right now. maybe there will be later.
}

void Tracker::onCreate(DataStore<Level::strong>& , int ){
    //there's nothing to do for a strong datastore right now. maybe there will be later.
}

void Tracker::onWrite(DataStore<Level::causal>&, int name, const Clock &version){
    //there's nothing to do for a strong datastore right now. maybe there will be later.	
}

void Tracker::onRead(DataStore<Level::causal>&, int name, const Clock &version){
	if (ends::prec(version,i->global_min)) {
		i->tracking.erase(name);
	}
	else i->tracking.emplace(name,version);
}
