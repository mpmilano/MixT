#pragma once
#include <memory>
#include <iostream>
#include <list>
#include <cassert>
#include "../extras"

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

namespace {
	constexpr int zero = __COUNTER__;
}
#define gen_id() sieve(__COUNTER__)

namespace Tracking {

	typedef long long TrackingSet;
	typedef long long TrackingId;

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

template<typename T, Tracking::TrackingId tid>
class TReadVal;

template<typename T, Tracking::TrackingSet st>
class IntermVal{
private:
	T internal;
	IntermVal(T &&t):internal(t){}
	static constexpr long long s = st;

public:

	template<Tracking::TrackingSet s_>
	auto touch(IntermVal<T, s_> &&t){
		using namespace Tracking;
		constexpr auto v = combine(s, sub(s_,s));
		return (IntermVal<T, v>) std::move(t);
	}

	template<Tracking::TrackingId id>
	IntermVal(const IntermVal<T,id> &rv):internal(rv.internal){}

	template<Tracking::TrackingSet s_>
	IntermVal<T,Tracking::combine(s,s_)> operator+(IntermVal<T,s_> v){
		return std::move(internal + v.internal);
	}

	template<typename F, typename G, typename... Args>
	//typename std::enable_if <is_stateless<F, Args...>::value && is_stateless<G, Args...>::value >::type
	void
	ifTrue(F f, G g, Args... rest) {
		//rest should be exact items we wish to use in the subsequent computation.
		//will just cast them all to themselves + this type
		if (internal) f(touch(std::move(rest))...);
		else g(touch(std::move(rest))...);
	}

	template<typename T_, Tracking::TrackingId tid>
	friend class TReadVal;

	template<typename T_, Tracking::TrackingSet s_>
	friend class IntermVal;

	void display(){
		std::cout << internal << std::endl;
	}

	static void displaySources(){
		for (auto e : Tracking::asList(s))
			std::cout << e << ",";
		std::cout << std::endl;
	}

};

template<typename T, Tracking::TrackingId tid>
class TReadVal : public IntermVal<T, tid>{

public:
	TReadVal(T t):IntermVal<T,tid>(std::move(t)){}
	static Tracking::TrackingId id() { return tid;}
};


#define ReadVal(a) TReadVal<a, gen_id()>

#define TIF3(a,f,g,b, c, d) {						\
		a.ifTrue([](decltype(a.touch(std::move(b))) b, \
			    decltype(a.touch(std::move(d))) d,		\
			    decltype(a.touch(std::move(c))) c) {f}	\
			 ,[](decltype(a.touch(std::move(b))) b, \
			     decltype(a.touch(std::move(d))) d, \
			     decltype(a.touch(std::move(c))) c) {g}, b, d, c ); }
