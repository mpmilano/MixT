#pragma once

#include "Tracker.hpp"

struct Tracker::timestamp {
	time_t& tv_sec;
	long& tv_nsec;
	void operator=(const timestamp&) = delete;
};
struct Tracker::timestamp_c {
	const time_t& tv_sec;
	const long& tv_nsec;
	operator bool() const;
	void operator=(const timestamp&) = delete;
};
static_assert(std::is_trivially_copyable<Tracker::timestamp>::value,"error: timestamp should be trivially copyable");

struct Tracker::Tombstone {
	Nonce nonce;
	int name() const;
};

struct Tracker::Ends : public ByteRepresentable{
private:
	std::vector<TrivialTriple<replicaID,  time_t, long> > contents;
	static_assert(std::is_trivially_copyable<TrivialTriple<replicaID, time_t, long > >::value,
				  "error: content's members should be trivially copyable");
	
	Ends(decltype(contents));
public:
	Ends() = default;
	timestamp_c at(replicaID) const;
	timestamp operator[](replicaID);
	bool prec(const Ends&) const;
	void fast_forward(const Ends&);
	static std::unique_ptr<Ends> merge(const std::vector<Ends const * >&);
	DEFAULT_SERIALIZATION_SUPPORT(Ends,contents);
};

struct Tracker::Metadata : public ByteRepresentable{
	Nonce nonce;
	CompactSet<read_pair> readSet;
	Ends ends;
	Metadata() = default;
	Metadata(decltype(nonce), decltype(readSet), decltype(ends));
	DEFAULT_SERIALIZATION_SUPPORT(Metadata,nonce,readSet,ends);
	
	static std::unique_ptr<Metadata> merge(const std::vector<Metadata const * >&){
		assert(false && "error: cannot Merge metadata structs");
	}
};
