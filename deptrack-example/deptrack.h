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

template<typename T, Tracking::TrackingId tid>
class TReadVal;

template<typename T, Tracking::TrackingSet... permitted>
class WriteVal;

template<typename T, Tracking::TrackingSet st>
class IntermVal{
private:
	T internal;
	IntermVal(T &&t):internal(t){}
	static constexpr long long s = st;

	template<typename T_, Tracking::TrackingSet s>
	static constexpr bool not_intermval_f(IntermVal<T_,s>*) { return false; }

	template<typename T_>
	static constexpr bool not_intermval_f(T_*) { return true; }

	template<typename T_>
	struct not_intermval : public std::integral_constant<bool,not_intermval_f( (T_*) nullptr )>::type {};

public:

	template<typename T_, Tracking::TrackingSet s_>
	auto touch(IntermVal<T_, s_> &&t){
		using namespace Tracking;
		constexpr auto v = combine(s, sub(s_,s));
		return (IntermVal<T_, v>) std::move(t);
	}

	template<typename T_, Tracking::TrackingId... ids>
	auto touch(WriteVal<T_,ids...> &&t) {
		using namespace Tracking;
		static_assert(contains(combine(ids...),st), "Invalid indirect flow detected!");
		return t;
	}

	template<Tracking::TrackingId id>
	IntermVal(const IntermVal<T,id> &rv):internal(rv.internal){}

#define allow_op(op) \
	template<Tracking::TrackingSet s_> \
	auto operator op (IntermVal<T,s_> v){ \
		auto tmp = internal  op  v.internal; \
		return IntermVal<decltype(tmp), Tracking::combine(s,s_)> (std::move(tmp)); \
	} \
\
	template<typename T_> \
	auto operator op (T_ v){ \
		static_assert(not_intermval<T_>::value, "Hey!  that's cheating!"); \
		auto tmpres = internal  op  v; \
		return IntermVal<decltype(tmpres),s> (std::move(tmpres)); \
	} \

	allow_op(-)
	allow_op(+)
	allow_op(*)
	allow_op(/)
	allow_op(==)
	allow_op(<)
	allow_op(>)

	template<typename F>
	auto f(F g){
		static_assert(is_stateless<F, T>::value, "No cheating!");
		auto res = g(internal);
		return IntermVal<decltype(res), s>(std::move(res));
	}



	template<typename F, typename G, typename... Args>
	//typename std::enable_if <is_stateless<F, strouch<Args>::type...>::value && is_stateless<G, stouch<Args>::type...>::value >::type
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
	static constexpr Tracking::TrackingId id() { return tid;}
};

template<typename T, Tracking::TrackingSet... permitted>
class WriteVal {
public:
	static constexpr Tracking::TrackingSet permset = Tracking::combine(permitted...);

	template<Tracking::TrackingSet cnds>
	void add(IntermVal<T, cnds>){
		static_assert(Tracking::subset(permset,cnds), "Error: id not allowed! Invalid Flow!");
	}
	template<Tracking::TrackingSet cnds>
	void put(IntermVal<T, cnds>){
		static_assert(Tracking::subset(permset,cnds), "Error: id not allowed! Invalid Flow!");
	}
	void incr(){}
};


#define ReadVal(a) TReadVal<a, gen_id()>

#define IDof(a) decltype(a)::id()

#define TranVals(int, a_balance, ids...) ReadVal(int) a_balance##_R = decltype(a_balance##_R)(100); \
	WriteVal<int,IDof(a_balance##_R), ##ids> a_balance##_W


#define TIF3(a,f,g,b, c, d) {						\
		a.ifTrue([](decltype(a.touch(std::move(b))) b, \
			    decltype(a.touch(std::move(d))) d,		\
			    decltype(a.touch(std::move(c))) c) {f}	\
			 ,[](decltype(a.touch(std::move(b))) b, \
			     decltype(a.touch(std::move(d))) d, \
			     decltype(a.touch(std::move(c))) c) {g}, b, d, c ); }


#define TIF2(a,f,g,b, c) {						\
		a.ifTrue([](decltype(a.touch(std::move(b))) b, \
			    decltype(a.touch(std::move(c))) c) {f}	\
			 ,[](decltype(a.touch(std::move(b))) b, \
			     decltype(a.touch(std::move(c))) c) {g}, b, c ); }
