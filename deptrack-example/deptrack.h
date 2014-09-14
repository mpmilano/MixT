#pragma once
#include <cmath>

constexpr int sieve(int ind){
	constexpr int limit = 10000;
	int primes[limit] = {};
	int z = 1;

	for (unsigned long long int i=2;i<limit;i++)
		primes[i]=1;

	for (unsigned long long int i=2;i<limit;i++)
		if (primes[i])
			for (unsigned long long int j=i;i*j<limit;j++)
				primes[i*j]=0;

	for (unsigned long long int i=2;i<limit;i++) 
		if (primes[i] && (z++ == ind) )
			return i;
	return 0;
}

constexpr int gcd(int a, int b){
	return b == 0 ? a : 
		(gcd(b, a % b) );
}

#define gen_id() sieve(__LINE__)

namespace Tracking {

	typedef int TrackingSet;
	typedef int TrackingId;

	constexpr TrackingSet combine(TrackingSet a, TrackingId b){
		return a * b;
	}
	constexpr bool contains(TrackingSet set, TrackingId member){
		return (set % member) == 0;
	}
	constexpr bool subset (TrackingSet super, TrackingSet sub){
		return (super % sub) == 0;
	}
	constexpr TrackingSet intersect (TrackingSet a, TrackingSet b){
		return gcd(a,b);
	}
	constexpr TrackingSet empty(){
		return 1;
	}
	constexpr TrackingSet singleton (TrackingId tt){
		return tt;
	}
};

template<typename T, Tracking::TrackingId tid>
class TReadVal{
public:
	static Tracking::TrackingId id() { return tid;}
};

#define ReadVal(a) TReadVal<a, gen_id()>
