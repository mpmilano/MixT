#include "DataStore.hpp"
#include <cstdlib>
#include <time.h>
#include "GDataStore.hpp"
#include "compile-time-lambda.hpp"
#include "utils.hpp"
#include <functional>

#include "Tracker_common.hpp"


using namespace std;

struct Tracker::Internals{
	Internals(const Internals&) = delete;
	DataStore<Level::strong> registeredStrong* {nullptr};
	std::unique_ptr<TrackerDSStrong > strongDS;
	typedef std::unique_ptr<TrackerDSStrong > (*getStrongInstance) (replicaID);
	getStrongInstance strongInst{nullptr};
	

	DataStore<Level::causal> registeredCausal* {nullptr};
	std::unique_ptr<TrackerDSCausal > causalDSmap;
	typedef std::unique_ptr<TrackerDSCausal > (*getCausalInstance) (replicaID);
	getCausalInstance causalInst{nullptr};

	Ends ends;
	std::set<read_pair> readSet;
	std::map<replicaID, std::unique_ptr<TrackerDSCausal> > replicaMap;

};

Tracker::Tracker():i{new Internals{}}{
	timespec ts;
	clock_gettime(CLOCK_REALTIME,&ts);
	srand(ts.tv_nsec);
}

Tracker::~Tracker(){
	delete i;
}

Tracker& Tracker::global_tracker(){
	static Tracker t;
	return t;
}

void Tracker::registerStore(DataStore<Level::strong>& ds, const TrackerDSStrong& wds, getStrongInstance f){
	assert(i->registeredStrong == nullptr);
	assert(i->strongDS.get() == nullptr);
	i->registeredStrong = &ds;
	i->strongDS.reset(heap_copy(wds));
	i->strongInst = f;
}
void Tracker::registerStore(DataStore<Level::strong>& ds, const TrackerDSCausal& wds, getCausalInstance f){
	assert(i->registeredCausal == nullptr);
	assert(i->causalDS.get() == nullptr);
	i->registeredCausal = &ds;
	i->causalDS.reset(heap_copy(wds));
	i->causalInst = f;
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

	bool is_lin_metadata(){
		assert(false && "TODO: unique name prefix that signifies metadata");
	}

	int make_lin_metaname(){
		assert(false &&
			   "TODO: whatever this is, make it compatible with is_metadata");
	}

}

namespace {
	write_lin_metadata(Tracker::Nonce nonce, Tracker::Internals& t){
		auto meta_name = make_lin_metaname(name);
		decltype(t.readSet) rscopy = t.readSet;
		rscopy.insert(std::make_pair(t.registeredStrong->instance_id(),nonce));
		t.strongDS->newMeta(t.strongDS->real,meta_name,
							{
								rscopy,
									nonce,
									t.registeredStrong->instance_id(),
									ends});
	}
	write_tombstone(Tracker::Nonce nonce, Tracker::Internals &i){
		const Tracker::Tombstone t {nonce};
		i.causalDS->newTomb(i.causalDS->real,t.name(), t);
	}
}

void Tracker::onWrite(DataStore<Level::strong>& ds_real, int name){
	assert(&ds_real == i->registeredStrong);
	auto &ds = *strongDS;
	if (!is_metadata(name)){
		auto nonce = rand();
		write_lin_metadata(nonce,*i);
		write_tombstone(nonce,*i);
	}
}

void Tracker::onRead(DataStore<Level::strong>& ds, int name){
	assert(&ds == i->registeredStrong);
	if (!is_metadata(name)){
		auto meta_name = make_metaname(name);
		if (i->strongDS->exists(ds,meta_name)){
			auto meta = i->strongDS->existingMeta(ds,meta_name).get();
			if (!meta.ends.prec(i->ends)){
				//TODO: argue more precisely about Ends and when to fast-forward it.
				i->ends.fast_forward(meta.ends);
				if (!i->causalDS->exists(i->causalDS->real,Tombstone{meta.nonce}.name())){
					i->readSet.insert(meta.readSet.begin(), meta.readSet.end());
					//TODO: if there's a way to request a sync, do it here.
					//we want to encounter that tombstone value as soon as possible.
				}
			}
		}
	}
}

void Tracker::tick(){
	//refresh the readset
	decltype(readSet) readSet_new;
	for (auto &p : readSet){
		if (!i->causalDS->exists(i->causalDS->real,Tombstone{p.second}.name())){
			readSet_new.insert(p);
		}
	}
	readSet = readSet_new;
}

namespace{
	auto generate_causal_metadata(Ends &tstamp, Internals& i) {
		for (auto &p : i.readSet){
			auto t = i.ends.at(p.fist);
			if (t) tstamp[p.first] = t;
		}
		auto natural_replica = i.registeredCausal->instance_id();
		auto t = i.ends.at(natural_replica);
		assert(t);
		tstamp[natural_replica] = t;
		return tstamp;
	}

	void write_causal_metadata(int name, Internals &i){
		Ends tstamp;
		//Note: this assumes that the causal + linearizable stores
		//have separate namespaces.  This is a reasonable assumption
		//in real life, but I have to remember it for local testing.
		auto meta_name = make_metaname(name);
		i.causalDS->newEnds(i.causalDS->real,meta_name,generate_causal_metadata(tstamp,i));
	}
}

void Tracker::onCreate(DataStore<Level::causal>& ds, int name){
	assert(&ds == registeredCausal);
	write_causal_metadata(name,*i);
}

void Tracker::onWrite(DataStore<Level::causal>&, int name){
	timespec ts;
	clock_gettime(CLOCK_REALTIME,&ts);
	i->ends[i->registeredCausal->instance_id()] = ts;
	write_causal_metadata(name,*i);
}

namespace{

	auto& fetch_replica(Internals& i, replicaID id){
		if (i.replicaMap.count(id) == 0){
			i.replicaMap.at(id).reset(getCausalInstance(id));
		}
		return i.replicaMap.at(id);
	}

	template<typename F1, typename F2>
	auto collect(Internals &i, const F1 &existing_obj, int name){
		std::vector<unique_ptr<GeneralRemoteObject> > relevant_ptrs;
		relevant_ptrs.emplace_back(existing_obj(i.causalDS->real,name));
		for (auto &p : i.readSet){
			auto &replica = fetch_replica(i,p.first);
			if (replica->exists(name)){
				relevant_ptrs.emplace_back(existing_obj(replica->real,name));
			}
		}
		return std::move(relevant_ptrs);
	}

}

void Tracker::onRead_internal(
	DataStore<Level::causal> &ds,int name,
	const std::function<std::unique_ptr<GeneralRemoteObject>
	(DataStore<Level::causal>&, int)>& existingT,
	const std::function<void (std::vector<std::unique_ptr<GeneralRemoteObject>>)>&
	mergeT,
	const std::function<std::unique_ptr<Ends> (std::vector<std::unique_ptr<GeneralRemoteObject>>)>&
	merge_ends
	){
	assert(&ds == i->registeredCausal);
	//mergeT should collect the value to return
	//back in header onRead
	mergeT(collect(*i,existingT,name));
	auto metaname = make_metaname(name);
	i->ends.fast_forward(*merge_ends(collect(*i,existingEnds,metaname)));
}
