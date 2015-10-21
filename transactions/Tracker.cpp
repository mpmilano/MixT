#include "DataStore.hpp"
#include <cstdlib>
#include <time.h>
#include "GDataStore.hpp"
#include "compile-time-lambda.hpp"
#include "utils.hpp"
#include <functional>

//Tracker.cpp, the entire file, can see private members of the Tracker family.
#define private public
#include "Tracker_common.hpp"
#undef private

using namespace std;

Tracker::Tracker(){
	timespec ts;
	clock_gettime(CLOCK_REALTIME,&ts);
	srand(ts.tv_nsec);
}

Tracker& Tracker::global_tracker(){
	static Tracker t;
	return t;
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

bool Tracker::registered(const GDataStore& gds) const{
	if (auto *t1 = dynamic_cast<const DataStore<Level::strong>* >(&gds)){
		return t1 == registeredStrong;
	}
	else if (auto *t2 = dynamic_cast<const DataStore<Level::causal>*  >(&gds)){
		return t2 == registeredCausal;
	}
	else assert(false && "there's a third kind of GDataStore?");
}

namespace {
	write_lin_metadata(Nonce nonce, Tracker& t){
		auto meta_name = make_lin_metaname(name);
		decltype(t.readSet) rscopy = t.readSet;
		rscopy.insert(std::make_pair(t.registeredStrong->instance_id(),nonce));
		t.strongDS->newMeta(meta_name,
							{
								rscopy,
									nonce,
									t.registeredStrong->instance_id(),
									ends});
	}
	write_tombstone(Nonce nonce, Tracker &t){
		const Tombstone t {nonce};
		t.causalDS.newTomb(t.name()?, t);
	}
}

void Tracker::onWrite(DataStore<Level::strong>& ds_real, int name){
	assert(&ds_real == registeredStrong);
	auto &ds = *strongDS;
	if (!is_metadata(name)){
		auto nonce = rand();
		write_lin_metadata(nonce,t);
		write_tombstone(nonce,t);
	}
}

void Tracker::onRead(DataStore<Level::causal>& ds, int name){
	assert(&ds == registeredStrong);
	if (!is_metadata(name)){
		auto meta_name = make_metaname(name);
		if (strongDS->exists(meta_name)){
			auto meta = strongDS->existingMeta(meta_name).get();
			if (!meta.ends.prec(ends)){
				//TODO: argue more precisely about Ends and when to fast-forward it.
				ends.fast_forward(meta.ends);
				if (!causalDS->exists(Tombstone{meta.nonce}.name())){
					readSet.insert(meta.readSet.begin(), meta.readSet.end());
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
		if (!causalDS->exists(Tombstone{p.second}.name())){
			readSet_new.insert(p);
		}
	}
	readSet = readSet_new;
}

void Tracker::onCreate(DataStore<Level::causal>&, int name){
	assert(&ds == registeredCausal);
	write_metadata();
}

void Tracker::onWrite(DataStore<Level::causal>&, int name);

void Tracker::onRead(DataStore<Level::strong>&, int name);
