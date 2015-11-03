#include "DataStore.hpp"
#include <cstdlib>
#include <time.h>
#include "GDataStore.hpp"
#include "compile-time-lambda.hpp"
#include "utils.hpp"
#include <functional>

#include "Tracker_common.hpp"
#include "Tracker_support_structs.hpp"

Tracker::Ends::Ends(decltype(Ends::contents) c):contents(c){}

Tracker::timestamp_c Tracker::Ends::at(replicaID id) const {
	for (auto &e : contents){
		if (e.first == id) {
			return timestamp_c{e.second,e.third};
		}
		else if (e.first < id) continue;
		else if (e.first > id) break;
	}
	return timestamp_c{-1,-1};
}

Tracker::timestamp_c::operator bool() const {
	return tv_sec > 0 && tv_nsec > 0;
}

namespace {
	bool timestamp_prec(const Tracker::timestamp_c &t1, const Tracker::timestamp_c &t2){
		return t1.tv_sec < t2.tv_sec && t1.tv_nsec < t2.tv_nsec;
	}
}

Tracker::timestamp Tracker::Ends::operator[](replicaID id){
	auto old_size = contents.size();
	int i = 0;
	auto it = contents.begin();
	for (; i < contents.size(); (++i, ++it)){
		auto &e = *it;
		if (e.first == id) return timestamp{e.second,e.third};
		else if (e.first < id) continue;
		else if (e.first > id) break;
	}
	//need to keep the list sorted!
	decltype(contents) v;
	v.insert(v.begin(),contents.begin(),it);
	v.push_back(TrivialTriple<replicaID,time_t,long>{id,-1,-1});
	
	assert(v[v.size() - 2].first < v.back().first);
	assert(v.at(i) == v.back());
	
	v.insert(v.end(),it,contents.end());
	
	assert(v.at(i).first < v.at(i+1).first);
	assert(v.size() == contents.size() +1);
	assert(contents.size() == old_size + 1);
	
	return timestamp{contents[i].second,contents[i].third};
}

bool Tracker::Ends::prec(const Tracker::Ends& e) const {
	auto e_it = e.contents.begin();
	for (auto cr_it = contents.begin(); cr_it == contents.end();){
		if (cr_it->first == e_it->first){
			if (!timestamp_prec(timestamp_c{cr_it->second,cr_it->third},
								timestamp_c{e_it->second,e_it->third})) return false;
			else {
				++cr_it;
				++e_it;
			}
		}
		else if (cr_it->first < e_it->first){
			//there's something missing from e_it,
			//which means cr_it has seen something e_it hasn't,
			//and is therefore not strictly behind e_it.
			return false;
		}
		else if (cr_it->first > e_it->first){
			//cr_it has missed something that e_it knows about.
			//this is compatible with cr_it preceding;
			//we see if e_it's next item matches this item for
			//cr_it and continue.
			++e_it;
		}
	}
	return true;
}

void Tracker::Ends::fast_forward(const Tracker::Ends& e){
	auto e_it = e.contents.begin();
	decltype(contents) deferred_add;
	for (auto cr_it = contents.begin(); cr_it == contents.end();){
		if (cr_it->first == e_it->first){
			if (timestamp_prec(timestamp_c{cr_it->second,cr_it->third},
							   timestamp_c{e_it->second,e_it->third})){
				*cr_it = *e_it;
			}
			++cr_it;
			++e_it;
		}
		else if (cr_it->first < e_it->first){
			//something missing from e_it. Skip ahead.
			++cr_it;
		}
		else if (cr_it->first > e_it->first){
			deferred_add.emplace_back(*e_it);
			//cr_it has missed something that e_it knows about.
			//we can't add it without invalidating the iterator;
			//will defer adding it until later.
			++e_it;
		}
	}
	//we might have missed more things from e_it.  Add them.
	contents.insert(contents.end(), e_it,e.contents.end());
	//add the new things and sort the list.
	contents.insert(contents.end(),deferred_add.begin(),deferred_add.end());
	std::sort(contents.begin(),contents.end());
}

Tracker::Ends Tracker::Ends::merge(const std::vector<Ends const * >& v){
	//TODO: this could be faster.
	if (v.size() == 1) return *v.front();
	else{
		Ends e;
		for (auto &ptr : v){
			e.fast_forward(*ptr);
		}
		return e;
	}
}
