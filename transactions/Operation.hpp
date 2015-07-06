#pragma once
#include "tuple_extras.hpp"
#include "../Backend.hpp"
#include "utils.hpp"
#include "../handle_utils"
#include "args-finder.hpp"
#include "ConStatement.hpp"
#include "fiter-varargs.hpp"

template<backend::Level l>
struct ConStatement;

//"Self" should always be the bottom of the inheritance.
template<backend::Level l, typename Self>
class Operation : public ConStatement<l>{
public:

	//Must have a static function "build" which creates the operation.
	//it should take first all handles, then all other arguments.

	static constexpr backend::Level level = l;

	BitSet<backend::HandleAbbrev> rs;
	
	Operation(const decltype(rs) &rs):rs(rs){}

	decltype(rs) getReadSet() const {return rs;}

	template<typename Handles, typename OtherArgs>
	static Self operate(const Handles &h,
						const OtherArgs &o,
						const BitSet<backend::HandleAbbrev> &){
		static_assert(forall_types<backend::is_handle, Handles>::value,"Error: must pass handles as initial arguments!");
		//TODO: read validation
		auto concat = std::tuple_cat(h,o);
		constexpr int numparams = std::tuple_size<Handles>::value +
			std::tuple_size<OtherArgs>::value;
		//auto f = convert(Self::build);
		//return callFunc(Self::build,concat,gens<numparams>::build());
		return callConstructor<Self>(concat,gens<numparams>::build());
	}
	
private:
	template<typename TupleSoFar>
	static auto make_tuples_(const TupleSoFar &ht, const BitSet<backend::HandleAbbrev> &bs){
		static_assert(is_tuple<TupleSoFar>::value,"Error! This isn't a tuple! bugs!");
		return std::make_tuple(std::get<0>(ht),std::get<1>(ht),bs);
	}
	
	template<typename TupleSoFar, backend::Client_Id id, backend::Level l2,
			 backend::HandleAccess ha2, typename T, typename... Rest>
	static auto make_tuples_(const TupleSoFar &ht,
							const backend::DataStore::Handle<id,l2,ha2,T> &fst, const Rest & ... r){
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

template<typename... T>
struct min_level;

//TODO: this is just supposed to be the min_level of the *sources*
//but right now it's the min level of all handles.  this is certainly
//"safe," but maybe I need to do something else?
template<typename Ret, typename... Args>
constexpr backend::Level oper_level(Ret (*) (Args...) ){
	return min_level<typename std::decay<Args>::type...>::value;
}



template<typename Ret, typename... Args>
BitSet<backend::HandleAbbrev> oper_readset(Ret (*) (Args...) ){
	//return fold_types<Func, filter<Args...>::type, std::integral_constant<int,0> >::value
	return 0;
}

//TODO: make this work when name of function isn't provided as x.
#define make_operation(Name, x) struct Name : public Operation<oper_level(x), Name> { \
		const backend::DataStore::Handle<1,Name::level, backend::HandleAccess::all, int> &h; \
		Name(decltype(h) h):Operation(oper_readset(x)),h(h){}			\
																		\
		auto operator()(){												\
			static const decltype(convert(x)) f = convert(x);			\
			return f(h);												\
		}																\
																		\
	}
