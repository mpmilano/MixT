#pragma once
#include "../Backend.hpp"
#include "utils.hpp"
#include "../handle_utils"

//"Self" should always be the bottom of the inheritance.
template<backend::Level l, backend::HandleAccess ha, typename Self>
class Operation {
public:

	//Must have a static function "build" which creates the operation.
	//it should take first all handles, then all other arguments.

	static constexpr backend::Level level = l;
	static constexpr backend::HandleAccess access = ha;

	template<typename Handles, typename OtherArgs>
	static Self operate(const Handles &h,
						const OtherArgs &o,
						const BitSet<backend::HandleAbbrev> &){
		//TODO: read validation
		auto concat = std::tuple_cat(h,o);
		constexpr int numparams = std::tuple_size<Handles>::value +
			std::tuple_size<OtherArgs>::value;
		return callFunc<Self>(Self::build,concat,gens<numparams>::build());
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

struct dummy_operation : public Operation<backend::Level::strong, backend::HandleAccess::all, dummy_operation>
{
	template<typename H,restrict(backend::is_handle<H>::value)>
	dummy_operation(const H&){}

	template<typename H,restrict(backend::is_handle<H>::value)>
	static dummy_operation build(H r){
		return r;
	}
};
