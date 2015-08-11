#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"
#include "CommonExprs.hpp"
#include <iostream>

#define if_concept(Cond,Then,Els)										\
	{static_assert(is_ConExpr<Cond>::value, "Error: if-condition not a condition"); \
	static_assert(is_cs_tuple<Then>::value, "Error: then-branch not a tuple of statements"); \
	static_assert(is_cs_tuple<Then>::value, "Error: else-branch not a tuple of statements");} \

#define if_concept_2(Cond,Then,Els)										\
	static_assert((get_level<Cond>::value == Level::causal &&			\
				   max_level<Then>::value == Level::causal &&			\
				   max_level<Els>::value == Level::causal)				\
				  ||													\
				  (get_level<Cond>::value == Level::strong),			\
				  "Error: implicit flow found in IF.")


template<typename Cond, typename Then, typename Els>
struct If : public ConStatement<min_level<typename min_level<Then>::type,
										  typename min_level<Els>::type >::value> {

	typedef Cond Cond_t;
	const Cond cond;
	const Then then;
	const Els els;

	If(const Cond& cond, const Then& then, const Els &els):
		cond(cond),then(then),els(els)
		{
			if_concept(Cond,Then,Els);
			if_concept_2(Cond,Then,Els);
		}

	BitSet<HandleAbbrev> getReadSet() const {
		assert(false && "TODO: split into strong + weak? ");
		//return set_union(get_ReadSet(cond),then.getReadSet(),els.getReadSet());
	}

	bool operator()(Store &s) const {
		static_assert(!is_ConStatement<decltype(run_ast(s,cond))>::value);
		static_assert(!is_ConStatement<decltype(call_all(s,then))>::value);
		return (run_ast(s,cond) ? call_all(s,then) : call_all(s,els));
	}
};

template<typename T>
T pick_useful(const T &t, std::nullptr_t){
	return t;
}

std::nullptr_t pick_useful(std::nullptr_t, std::nullptr_t){
	return nullptr;
}

template<typename T>
T pick_useful(std::nullptr_t, const T& t){
	return t;
}

template<typename T, typename U>
T pick_useful(const U& u, const T& t){
	return t;
}

template<unsigned long long ID, typename Cond, typename Then, typename Els>
auto find_usage(const If<Cond,Then,Els>& _if){
	return fold(std::tuple_cat(std::make_tuple(_if.cond), _if.then, _if.els),
				[](const auto &e, const auto &acc){
					return pick_useful(find_usage<ID>(e),acc);
				}
				, nullptr);
}



template<typename A, typename B, typename C>
constexpr bool is_If_f(const If<A,B,C>*){
	return true;
}

template<typename A>
constexpr bool is_If_f(const A*){
	return false;
}

template<typename T>
struct is_If : std::integral_constant<bool,is_If_f(mke_p<T>())>::type {};


template<typename Cond, typename Then, typename Els>
std::ostream & operator<<(std::ostream &os, const If<Cond,Then,Els>& i){
	return os << "(" << i.cond <<" ? " << i.then << ")";
}
