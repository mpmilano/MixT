#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"
#include "Seq.hpp"
#include <iostream>


template<typename Cond, typename Then, typename Els>
class If;

#define handle_level backend::handle_level

#define if_concept(Cond,Then,Els) ( \
																		\
		is_ConStatement<Then>::value && is_ConStatement<Els>::value		\
		&& is_ConExpr<Cond>::value										\
																		\
		)

#define if_concept_2(Cond,Then,Els) \
	((get_level<Cond>::value == Level::causal &&						\
	  get_level<Then>::value == Level::causal &&						\
	  get_level<Els>::value == Level::causal)							\
	 ||																	\
	 (get_level<Cond>::value == Level::strong))

template<typename Cond, typename Then, typename Els,
		 restrict(if_concept(Cond,Then,Els))>
If<Cond, Then, Els> make_if(const Cond& c, const Then &t, const Els &e){
	static_assert(if_concept_2(Cond,Then,Els), "Failure: consistency violation.");
	return If<Cond,Then,Els>(c,t,e);
}

template<typename Cond, typename Then1, typename Then2, typename Els1, typename Els2>
auto make_if(const Cond& c, const Seq<Then1, Then2> &t, const Seq<Els1, Els2> &e){
	return tuple_2fold(
		[&](const auto &a, const auto &b, const auto &acc){
			auto acc2 = acc.operator,(make_if(c,a,b));
			return acc2;
		},t.strong,e.strong,empty_seq());
}


template<typename Cond, typename Then, typename Els>
class If : public ConStatement<get_level<Cond>::value> {
public:
	static constexpr Level level = get_level<Cond>::value;
private:
	const Cond cond;
	const Then then;
	const Els els;
	
	If(const Cond& cond, const Then& then, const Els &els):
		cond(cond),then(then),els(els)
		{
			static_assert(if_concept(Cond,Then,Els),"Bad types got to constructor");
		}

private:

	template<typename Next, restrict((get_level<Next>::value == level
									  || get_level<Next>::value == Level::causal)
									 && is_ConStatement<Next>::value)>
	auto comma_op(const Next& n,
				  const ConStatement<get_level<Next>::value>&) const{
		return make_if(cond, (make_seq(then),n), (make_seq(els),n));
	}

	template<typename Next, restrict(get_level<Next>::value != level
									 && get_level<Next>::value == Level::strong
									 && is_ConStatement<Next>::value)>
	auto comma_op(const Next& n, const ConStatement<Level::strong>& ) const{
		return make_seq(n);
	}

	static_assert(!is_base_CS<If>::value, "something is very wrong");
	
public:

	template<typename Next>
	auto operator,(const Next &n) const {
		return comma_op(n,n);
	}
	
	template<typename Cond2, typename Then2, typename Els2, typename ignore>
	friend If<Cond2,Then2,Els2> make_if(const Cond2& , const Then2 &, const Els2 &);

	template<typename Cond2, typename Then2, typename Els2>
	friend std::ostream & operator<<(std::ostream &os, const If<Cond2,Then2,Els2>& i);
};

template<typename Cond, typename Then, typename Els>
std::ostream & operator<<(std::ostream &os, const If<Cond,Then,Els>& i){
	return os << "(condition ? " << i.then << ":" << i.els << ")";
}
