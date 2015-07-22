#pragma once
#include "tuple_extras.hpp"
#include "utils.hpp"
#include "args-finder.hpp"
#include "ConStatement.hpp"
#include "filter-varargs.hpp"

template<Level l>
struct ConStatement;


template<typename... T>
struct min_level;

//"Self" should always be the bottom of the inheritance.
template<Level l, typename Self>
class Operation : public ConStatement<l>{
public:

	//Must have a static function "build" which creates the operation.
	//it should take first all handles, then all other arguments.

	static constexpr Level level = l;

	BitSet<HandleAbbrev> rs;
	
	Operation(const decltype(rs) &rs):rs(rs){}

	decltype(rs) getReadSet() const {return rs;}

	template<typename Handles, typename OtherArgs>
	static Self operate(const Handles &h,
						const OtherArgs &o,
						const BitSet<HandleAbbrev> &bs){
		static_assert(forall_types<is_handle, Handles>::value,"Error: must pass handles as initial arguments!");
		static_assert(forall_types<is_not_handle, OtherArgs>::value,"Error: 'other arguments' for operation contains a handle!");
		static_assert(min_level<Handles>::value == l, "Error: attempt to declare operation at level incompatible with handle arguments!");
		//TODO: read validation
		return Self(h,o,bs); 
	}
	
private:
	template<typename TupleSoFar>
	static auto make_tuples_(const TupleSoFar &ht, const BitSet<HandleAbbrev> &bs){
		static_assert(is_tuple<TupleSoFar>::value,"Error! This isn't a tuple! bugs!");
		return std::make_tuple(std::get<0>(ht),std::get<1>(ht),bs);
	}
	
	template<typename TupleSoFar,  Level l2,
			 HandleAccess ha2, typename T, typename... Rest>
	static auto make_tuples_(const TupleSoFar &ht,
							const Handle<l2,ha2,T> &fst, const Rest & ... r){
		static_assert(is_tuple<TupleSoFar>::value,"Error! This isn't a tuple! bugs!");
		using namespace std;
		auto next = make_tuple(tuple_cat(get<0>(ht),make_tuple(fst)),get<1>(ht),get<2>(ht));
		return make_tuples_(next,r...);
	}
public:
	template<typename... Rest>
	static auto make_tuples(const Rest & ... r){
		using namespace std;
		return make_tuples_(make_tuple(make_tuple(),make_tuple(),make_tuple()),r...);
	}


};

//TODO: this is just supposed to be the min_level of the *sources*
//but right now it's the min level of all handles.  this is certainly
//"safe," but maybe I need to do something else?
template<typename Ret, typename... Args>
constexpr Level oper_level(Ret (*) (Args...) ){
	return min_level<typename std::decay<Args>::type...>::value;
}

template<typename T,restrict(!std::is_function<T>::value)>
constexpr Level oper_level(const T&){
	static_assert(!std::is_function<T>::value,"Error: oper_level only functions on function pointers!");
	return Level::strong;
}

template<typename... Handles>
BitSet<HandleAbbrev> oper_readset(const std::tuple<Handles...> &h){
	return fold<BitSet<HandleAbbrev> >
		(h,
		 [](const auto &h1, auto bs){
			return (canRead(h1.ha) ? bs.insert(h1.abbrev()) : bs);
		},0);
}

template<typename Ret, typename... Args>
constexpr auto oper_handles_f(Ret (*) (Args...) ){
	return mke<typename filter<is_handle,Args...>::type>();
}

template<typename T>
constexpr std::tuple<> oper_handles_f(const T& ){
	return mke<std::tuple<> >();
}

template<typename Ret, typename... Args>
constexpr auto oper_other_f(Ret (*) (Args...) ){
	return mke<typename filter<is_not_handle,Args...>::type>();
}

//TODO: make this work when name of function isn't provided as x.
//TODO: now assumes that consistency level will be a template parameter.
//this is probably a weirdly specific assumption.
//TODO: operator() always returns true now.  Should theoretically
//expose some mechanism to propogate failure up to this boolean.
//TODO: not exposing access to the temporary store here.
//If I choose to track handle modifications via it, then this is bad.

#define make_operation_lvl(Name,x) struct Name : public Operation<oper_level(x), Name> { \
		const decltype(oper_handles_f(x)) hndls;						\
		const decltype(oper_other_f(x)) other;							\
																		\
		Name(const decltype(hndls) &h,									\
			 const decltype(other) &o, BitSet<HandleAbbrev> bs):		\
			Operation(oper_readset(h).addAll(bs)),hndls(h),other(o){}	\
																		\
		bool operator()(Store &) const{									\
			static const decltype(convert(x)) f = convert(x);			\
			auto concat = std::tuple_cat(hndls,other);					\
			constexpr int numparams = std::tuple_size<decltype(hndls)>::value + \
				std::tuple_size<decltype(other)>::value;				\
			callFunc(f,concat,gens<numparams>::build());				\
			return true;												\
		}																\
																		\
	}

#define make_operation(Name, x) make_operation_lvl(Name ## _strong,x<Level::strong>); \
	make_operation_lvl(Name ## _causal,x<Level::causal>);		\
	template<Level l>											\
	using Name = typename std::conditional<l == Level::causal, Name ## _causal, Name ## _strong >::type

