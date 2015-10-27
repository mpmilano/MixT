#pragma once

template<Level l>
struct TrackerDS {
	
	DataStore<l> &real;
	Handle<l, HandleAccess::all, Tracker::Ends> (*newEnds) (DataStore<l>&, int, const Tracker::Ends&);
	Handle<l, HandleAccess::all, Tracker::Metadata> (*newMeta) (DataStore<l>&, int, const Tracker::Metadata&);
	Handle<l, HandleAccess::all, Tracker::Tombstone> (*newTomb) (DataStore<l>&, int, const Tracker::Tombstone&);
	bool (*exists) (DataStore<l>&, int);
	Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*existingMeta) (DataStore<l>&, int);

	TrackerDS(
			DataStore<l> &real,
			Handle<l, HandleAccess::all, Tracker::Ends> (*newEnds) (DataStore<l>&, int, const Tracker::Ends&),
			Handle<l, HandleAccess::all, Tracker::Metadata> (*newMeta) (DataStore<l>&, int, const Tracker::Metadata&),
			Handle<l, HandleAccess::all, Tracker::Tombstone> (*newTomb) (DataStore<l>&, int, const Tracker::Tombstone&),
			bool (*exists) (DataStore<l>&, int),
			Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*existingMeta) (DataStore<l>&, int)
		):
		real(real),
		newEnds(newEnds),
		newMeta(newMeta),
		newTomb(newTomb),
		exists(exists),
		existingMeta(existingMeta) {}

	template<typename A, typename B, typename C, typename D, typename E, typename F>
	TrackerDS(const std::tuple<A *,B *,C *,D* ,E* ,F *> &a)
		:TrackerDS(*std::get<0>(a),*std::get<1>(a),*std::get<2>(a),*std::get<3>(a),*std::get<4>(a),*std::get<5>(a)){}
};

/*

struct Tracker::TrackerDSStrong : public TrackerDS<Level::strong> {
	TrackerDSStrong(DataStore<Level::strong> &real,
					Handle<Level::strong, HandleAccess::all, Tracker::Ends> (*newEnds) (DataStore<Level::strong>&, int, const Tracker::Ends&),
					Handle<Level::strong, HandleAccess::all, Tracker::Metadata> (*newMeta) (DataStore<Level::strong>&, int, const Tracker::Metadata&),
					Handle<Level::strong, HandleAccess::all, Tracker::Tombstone> (*newTomb) (DataStore<Level::strong>&, int, const Tracker::Tombstone&),
					bool (*exists) (DataStore<Level::strong>&, int),
					Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*existingMeta) (DataStore<Level::strong>&, int)
)
	:TrackerDS<Level::strong>(real,newEnds, newMeta, newTomb, exists, existingMeta) {}
		
};

struct Tracker::TrackerDSCausal : public TrackerDS<Level::causal> {
	TrackerDSCausal(DataStore<Level::causal> &real,
					Handle<Level::causal, HandleAccess::all, Tracker::Ends> (*newEnds) (DataStore<Level::causal>&, int, const Tracker::Ends&),
					Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*newMeta) (DataStore<Level::causal>&, int, const Tracker::Metadata&),
					Handle<Level::causal, HandleAccess::all, Tracker::Tombstone> (*newTomb) (DataStore<Level::causal>&, int, const Tracker::Tombstone&),
					bool (*exists) (DataStore<Level::causal>&, int),
					Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*existingMeta) (DataStore<Level::causal>&, int)
)
	:TrackerDS<Level::causal>(real,newEnds, newMeta, newTomb, exists, existingMeta) {}
		
};

Tracker::TrackerDSStrong wrapStore(
		DataStore<Level::strong> &real,
		Handle<Level::strong, HandleAccess::all, Tracker::Ends> (*newEnds) (DataStore<Level::strong>&, int, const Tracker::Ends&),
		Handle<Level::strong, HandleAccess::all, Tracker::Metadata> (*newMeta) (DataStore<Level::strong>&, int, const Tracker::Metadata&),
		Handle<Level::strong, HandleAccess::all, Tracker::Tombstone> (*newTomb) (DataStore<Level::strong>&, int, const Tracker::Tombstone&),
		bool (*exists) (DataStore<Level::strong>&, int),
		Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*existingMeta) (DataStore<Level::strong>&, int) 
		){
	return  Tracker::TrackerDSStrong{
		real,newEnds,newMeta,newTomb,exists,existingMeta};
}

Tracker::TrackerDSCausal wrapStore(
		DataStore<Level::causal> &real,
		Handle<Level::causal, HandleAccess::all, Tracker::Ends> (*newEnds) (DataStore<Level::causal>&, int, const Tracker::Ends&),
		Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*newMeta) (DataStore<Level::causal>&, int, const Tracker::Metadata&),
		Handle<Level::causal, HandleAccess::all, Tracker::Tombstone> (*newTomb) (DataStore<Level::causal>&, int, const Tracker::Tombstone&),
		bool (*exists) (DataStore<Level::causal>&, int),
		Handle<Level::causal, HandleAccess::all, Tracker::Metadata> (*existingMeta) (DataStore<Level::causal>&, int) 
		){
	return  Tracker::TrackerDSCausal{
		real,newEnds,newMeta,newTomb,exists,existingMeta};
}
*/
