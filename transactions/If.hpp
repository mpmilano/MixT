#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"
#include "Seq.hpp"
#include <iostream>


template<typename Cond, typename Then>
class If;

#define handle_level backend::handle_level

#define if_concept(Cond,Then) ( \
																		\
		is_ConStatement<Then>::value									\
		&& is_ConExpr<Cond>::value										\
																		\
		)

#define if_concept_2(Cond,Then) \
	((get_level<Cond>::value == Level::causal &&						\
	  get_level<Then>::value == Level::causal)							\
	 ||																	\
	 (get_level<Cond>::value == Level::strong))

template<typename Cond, typename Then,
		 restrict(if_concept(Cond,Then))>
If<Cond, Then> make_if(const Cond& c, const Then &t){
	static_assert(if_concept_2(Cond,Then), "Failure: consistency violation.");
	return If<Cond,Then>(c,t);
}

template<typename Cond, typename Then1, typename Then2>
auto make_if(const Cond& c, const Seq<Then1, Then2> &t){
	return fold(t.strong,
				[&](const auto &a, const auto &acc){
					auto acc2 = acc.operator/(make_if(c,a));
					return acc2;
				},empty_seq()) /
		fold(t.weak,
			 [&](const auto &a, const auto &acc){
				 auto acc2 = acc.operator/(make_if(c,a));
				 return acc2;
			 },empty_seq());
}


template<typename Cond2, typename Then1, typename Then2>
auto make_if(const Cond2& c,
			 const std::initializer_list<Seq<Then1, Then2> > &t){
	return make_if(c,*(t.begin()));
}

template<typename Els>
struct Else {
	const Els &e;
};

template<typename Els>
auto make_else(const Els &e){
	Else<Els> ret{e};
	return ret;
}

template<typename Els, typename Str, typename... Stuff>
auto operator/(const Seq<Str, std::tuple<Stuff...> > &s, const Else<Els> & e){
	typedef typename last_of<Stuff...>::type LastIf;
	static constexpr int last_index = sizeof...(Stuff) - 1;
	return s / make_if(
		Not<typename LastIf::Cond_t>(
			std::get<last_index>(s.weak).cond)
		,e);
}



template<typename Cond, typename Then>
class If : public ConStatement<get_level<Then>::value> {
public:
	static constexpr Level level = get_level<Then>::value;
	typedef Cond Cond_t;
	const Cond cond;
	const Then then;
private:
	If(const Cond& cond, const Then& then):
		cond(cond),then(then)
		{
			static_assert(if_concept(Cond,Then) && if_concept_2(Cond,Then),
						  "Bad types got to constructor");
		}		
public:

	CONNECTOR_OP

	template<typename Els>
	auto operator/(const Else<Els> &e) const {
		return make_seq(*this) / make_if(Not<Cond>(cond),e);
	}

	BitSet<backend::HandleAbbrev> getReadSet() const {
		return set_union(get_ReadSet(cond),then.getReadSet());
	}

	auto operator()() const {
		return (cond() ? then() : Noop<level>().operator()());
	}

	
	template<typename Cond2, typename Then2, typename ignore>
	friend If<Cond2,Then2> make_if(const Cond2& , const Then2 &);

	template<typename Cond2, typename Then2>
	friend std::ostream & operator<<(std::ostream &os, const If<Cond2,Then2>& i);
};

template<typename Cond, typename Then>
std::ostream & operator<<(std::ostream &os, const If<Cond,Then>& i){
	return os << "(condition ? " << i.then << ")";
}

#define IF make_if(
#define THEN ,
#define ELSE ) / make_else(
#define FI ) /
