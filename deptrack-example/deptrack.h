#pragma once
#include <memory>
#include <iostream>
#include <list>
#include <cassert>
#include <utility>
#include "tracking.h"
#include "../extras"

template<typename T, Tracking::TrackingId tid>
class ReadVal;

template<typename T, Tracking::TrackingSet... permitted>
class WriteVal;

template<typename T, Tracking::TrackingId s, Tracking::TrackingSet... sets>
struct TransVals;

template<typename T, Tracking::TrackingSet st>
class IntermVal{
private:
	T internal;
	IntermVal(T &&t):internal(t),r(*this){}
	static constexpr long long s = st;

	template<typename T_, Tracking::TrackingSet s>
	static constexpr bool not_intermval_f(IntermVal<T_,s>*) { return false; }

	template<typename T_>
	static constexpr bool not_intermval_f(T_*) { return true; }

	template<typename T_>
	struct not_intermval : public std::integral_constant<bool,not_intermval_f( (T_*) nullptr )>::type {};

public:

	IntermVal &r;

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

	template<typename T_, Tracking::TrackingId id, Tracking::TrackingId... ids>
	auto touch(TransVals<T_,id,ids...> &&t) {
		using namespace Tracking;
		static_assert(contains(combine(id,ids...),st), "Invalid indirect flow detected!");
		return t;
	}


	template<Tracking::TrackingId id>
	IntermVal(const IntermVal<T,id> &rv):internal(rv.internal),r(*this){}

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
	friend class ReadVal;

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
class ReadVal : public IntermVal<T, tid>{

public:
	ReadVal(T t):IntermVal<T,tid>(std::move(t)){}
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


#define IDof(a) decltype(a)::id()

template<typename T, Tracking::TrackingId s, Tracking::TrackingSet... sets>
struct TransVals : public std::pair<ReadVal<T,s>, WriteVal<T,s,sets...> > {
	static constexpr Tracking::TrackingId id() {return s;};
	ReadVal<T,id()>& r() {return this->first;}
	WriteVal<T,id(),sets...>& w() {return this->second;}

	TransVals(int init_val):
		std::pair<ReadVal<T,id()>, WriteVal<T,id(),sets...> >(
			ReadVal<T,id()>(init_val),
			WriteVal<T,id(),sets...>()){}

	operator ReadVal<T,id()>& () {return this->first;}
	operator WriteVal<T,id(),sets...>& () {return this->second;}

#define forward_read(op)			   \
	template<typename T_>			     \
	auto op (T_ v){return r().op(v);}

#define op_read(op) forward_read(operator op)

#define forward_write(op)			   \
	template<typename T_>			     \
	auto op (T_ v){return w().op(v);}

#define op_write(op) forward_write(operator op)

	op_read(>)
	op_read(+)
	op_read(-)
	op_read(/)
	op_read(*)
	op_read(<)
	op_read(==)
	forward_write(put)
	forward_write(add)

};

#define TranVals(T, ids...) TransVals<T,gen_id(), ##ids>

//so it would be really cool to hack clang and all, but I think that
//these macros will work fine for the prototype stages.

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

#define TIF(a,f,g,b) {						\
		a.ifTrue([](decltype(a.touch(std::move(b))) b) {f},	\
			 [](decltype(a.touch(std::move(b))) b) {g}, b); }
