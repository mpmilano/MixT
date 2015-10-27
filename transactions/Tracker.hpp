//What goes here: managed tracking of stores, explicitly for
//cross-store stuff.

#pragma once

#include "CompactSet.hpp"
#include "GDataStore.hpp"
#include "compile-time-lambda.hpp"
#include "utils.hpp"
#include <functional>
#include <time.h>
#include "TrivialPair.hpp"

template<Level l, HandleAccess HA, typename T>
struct Handle;

template<Level l>
struct TrackerDS;

class Tracker {
public:
	using TrackerDSStrong = TrackerDS<Level::strong>;
	using TrackerDSCausal = TrackerDS<Level::causal>;
	using replicaID = int;
	static_assert(std::is_trivially_copyable<replicaID>::value,"error: replicaID should be trivially copyable");
	using Nonce = int;
	using read_pair = TrivialPair<replicaID, Nonce>;
	static_assert(std::is_trivially_copyable<read_pair>::value,"error: read_pair should be trivially copyable");
	using timestamp = timespec;
	static_assert(std::is_trivially_copyable<timestamp>::value,"error: timestamp should be trivially copyable");
	typedef std::unique_ptr<TrackerDSStrong > (*getStrongInstance) (replicaID);
	typedef std::unique_ptr<TrackerDSCausal > (*getCausalInstance) (replicaID);
	
	struct Ends : public ByteRepresentable{
	private:
		std::vector<TrivialTriple<replicaID,  time_t, long> > contents;
		static_assert(std::is_trivially_copyable<TrivialTriple<replicaID, time_t, long > >::value,
					  "error: content's members should be trivially copyable");

		Ends(decltype(contents));
	public:
		Ends() = default;
		const timestamp& at(replicaID) const;
		const timestamp* at_p(replicaID) const;
		timestamp& operator[](replicaID);
		bool prec(const Ends&) const;
		void fast_forward(const Ends&);
		static Ends merge(const std::vector<std::unique_ptr<Ends> >&);
		DEFAULT_SERIALIZATION_SUPPORT(Ends,contents);
	};
	struct Tombstone {
		Nonce nonce;
		int name() const;
	};
	struct Metadata : public ByteRepresentable{
		Nonce nonce;
		CompactSet<read_pair> readSet;
		Ends ends;
		Metadata() = default;
		Metadata(decltype(nonce), decltype(readSet), decltype(ends));
		DEFAULT_SERIALIZATION_SUPPORT(Metadata,nonce,readSet,ends);
	};
	
	struct Internals;
private:
	Internals *i;

	
public:
	static Tracker& global_tracker();

	bool registered(const GDataStore&) const;

	void registerStore(DataStore<Level::strong> &, std::unique_ptr<TrackerDSStrong>, getStrongInstance);
	void registerStore(DataStore<Level::causal> &, std::unique_ptr<TrackerDSCausal>, getCausalInstance);

	template<typename DS, typename Ret>
	void registerStore(DS &ds, Ret (*f) (replicaID));

	void tick();
	
	void onWrite(DataStore<Level::strong>&, int name);

	void onWrite(DataStore<Level::causal>&, int name);

	void onCreate(DataStore<Level::causal>&, int name);
	
	void onRead(DataStore<Level::strong>&, int name);

private:
	void onRead_internal(
		DataStore<Level::causal>&,
		int name,
		const std::function<std::unique_ptr<GeneralRemoteObject> (
			DataStore<Level::causal>&, int)> &
		existingT,
		const std::function<void (std::vector<std::unique_ptr<GeneralRemoteObject>>)>&
		mergeT,
		const std::function<std::unique_ptr<GeneralRemoteObject> (
							 DataStore<Level::causal>&, int)> &
		existingEnds,
		const std::function<std::unique_ptr<Ends> (std::vector<std::unique_ptr<GeneralRemoteObject>>)>&
		mergeEnds
		);
public:

	template<typename Vec>
	static auto default_merge(const Vec &v){
		return Vec::value_type::merge(map(v,[](auto &a){return a->get();}));
	}

	//need to know the type of the object we are writing here.
	//thus, a lot of this work needs to get done in the header (sadly).
	template<typename T, template<typename> class RO, typename DS>
	auto onRead(DS& ds, int name,
				const std::function<std::unique_ptr<T> (std::vector<std::unique_ptr<RO<T> > >)> &merge = default_merge,
				const std::function<std::unique_ptr<Ends> (std::vector<std::unique_ptr<RO<Ends> > >)> &mergeEnds = default_merge){
		static_assert(std::is_base_of<DataStore<Level::causal>,DS>::value,
			"Error: first argument must be a DataStore");
		static_assert(std::is_base_of<RemoteObject<T> ,RO<T> >::value,
			"Error: RO must be *your* RemoteObject type");
		
		static auto existingEnds = [](auto &_ds, auto name){
			auto &ds = dynamic_cast<DS&>(_ds);
			return std::unique_ptr<GeneralRemoteObject>(ds.template existingRaw<Ends>(name).release());
		};
		static auto existingT = [](auto &_ds, auto name) {
			auto &ds = dynamic_cast<DS&>(_ds);
			return std::unique_ptr<GeneralRemoteObject>(ds.template existingRaw<T>(name).release());
		};
		static auto castBack = [](std::unique_ptr<GeneralRemoteObject> p) -> std::unique_ptr<RO<T> > {
			if (auto *pt = dynamic_cast<RO<T>* >(p.release()))
				return std::unique_ptr<RO<T> >(pt);
			else assert(false && "Error casting from GeneralRemoteObject back to specific RO impl");
		};
		
		std::unique_ptr<T> ret;
		onRead_internal(ds,name,existingT,
						[&](const auto &v){ret.reset(merge(map(v,castBack)));},
						existingEnds,
						[&](const auto &v){return mergeEnds(map(v,castBack));});
		return ret;
	}

	Tracker();
	virtual ~Tracker();

	Tracker(const Tracker&) = delete;
};


