#pragma once

namespace {
	constexpr int zero = __COUNTER__;
}
#define gen_id() ctm::sieve(__COUNTER__)

namespace ctm {
	constexpr int sieve(int ind0){
		constexpr int limit = 10000;
		int ind = ind0 % limit;
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
}

namespace Tracking {

	using namespace ctm;

	typedef long long TrackingSet;
	typedef long long TrackingId;

	constexpr TrackingSet combine(TrackingSet a){
		return a;
	}

	template<typename... Args>
	constexpr typename std::enable_if<sizeof...(Args) != 0, TrackingSet>::type combine(TrackingSet a, Args... b){
		return a * combine(b...);
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

	constexpr TrackingSet sub (TrackingSet big, TrackingSet small){
		auto v = intersect(big,small);
		return big/v;
	}

	std::list<TrackingId> asList(TrackingSet s){
		//really slow
		TrackingSet curr = s;
		std::list<TrackingId> ret;
		for (int i = 1; curr != 1; ++i){
			auto s = sieve(i);
			if (contains(curr, s)){
				ret.push_back(s);
				curr /= s;
				while (contains(curr,s)) curr /= s;
			}
		}
		return ret;
	}
};

