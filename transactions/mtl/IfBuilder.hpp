#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"
#include "CommonExprs.hpp"
#include "TransactionBuilder.hpp"
#include "If.hpp"

namespace myria { namespace mtl {


		template<typename PrevBuilder, typename Cond, typename Then, typename Els>
		struct IfBuilder : public Base_Builder{
			const PrevBuilder prevBuilder;
			const If<Cond,Then,Els > this_if;
			typedef typename PrevBuilder::pc old_pc;
			typedef std::integral_constant<
				Level, min_level<old_pc,Cond>::value> pc;
			IfBuilder(const PrevBuilder &pb, const If<Cond,Then,Els> &to)
				:prevBuilder(pb),this_if(to){
				static_assert(is_ConExpr<Cond>::value,
							  "Error: If condition is not an expression.");
				static_assert(can_flow(PrevBuilder::pc::value,get_level<Cond>::value),
							  "Error: Flow violation in declaration of If."
					);
				static_assert(is_builder<PrevBuilder>(), "Error: PrevBuilder is not a builder!");
				//Note to self: the if_concept takes care of flows from
				//the condition to the branches.
				//that *can* be moved here if we want (weak TODO).
			}
		};

		template<typename PrevBuilder, typename Cond, typename Then, typename Els>
		constexpr bool is_IfBuilder_f(const IfBuilder<PrevBuilder, Cond, Then, Els>*) {
			return true;
		}


		template<typename T>
		struct is_IfBuilder : std::integral_constant<bool, is_IfBuilder_f(mutils::mke_p<T>()) >::type {};


		template<typename PrevBuilder, typename Cond, typename Then>
		struct ThenBuilder : IfBuilder<PrevBuilder,Cond,Then,std::tuple<> >{
			ThenBuilder(const PrevBuilder &pb, const If<Cond,Then,std::tuple<>> &to)
				:IfBuilder<PrevBuilder,Cond,Then,std::tuple<> >(pb,to) {
				static_assert(is_builder<PrevBuilder>(), "Error: PrevBuilder is not a builder!");
			}

			template<typename T>
			auto operator/(const T &t) const {
				static_assert(is_ConStatement<T>::value,
							  "Error: non-statement in Then clause of If.");
				typedef mutils::Cat<Then,std::tuple<T> > newThen;
				If<Cond,newThen,std::tuple<> >
					new_if(this->this_if.cond,
						   std::tuple_cat(this->this_if.then,
										  std::make_tuple(t)),
						   std::tuple<>());
				ThenBuilder<PrevBuilder,Cond,newThen >
					r(this->prevBuilder,new_if);
				return r;
			}
		};

		template<typename PrevBuilder, typename Cond, typename Then, typename Els>
		struct ElseBuilder : IfBuilder<PrevBuilder,Cond,Then, Els>{
			ElseBuilder(const PrevBuilder &pb, const If<Cond,Then,Els> &to)
				:IfBuilder<PrevBuilder,Cond,Then,Els>(pb,to) {}

			template<typename T>
			auto operator/(const T &t) const {
				static_assert(is_ConStatement<T>::value, "Error: non-statement in Else clause of If.");
				typedef mutils::Cat<Els,std::tuple<T> > newEls;
				If<Cond,Then,newEls >
					new_if(this->this_if.cond,this->this_if.then, std::tuple_cat(this->this_if.els, std::make_tuple(t)));
				ElseBuilder<PrevBuilder,Cond,Then,newEls > r(this->prevBuilder,new_if);
				return r;
			}
		};

		template<typename Cond>
		struct IfBegin {
			const Cond c;
		};

		struct IfEnd { constexpr IfEnd(){} };
	
		template<typename PrevBuilder, typename Cond>
		auto append(const PrevBuilder &pb, const IfBegin<Cond> &ib){
			If<Cond,std::tuple<>,std::tuple<> > if_elem(ib.c,std::tuple<>(),std::tuple<>());
			ThenBuilder<PrevBuilder, Cond, std::tuple<> > r(pb,if_elem);
			return r;
		}

		//TODO: else. Support doesn't even exist in the macros right now.
		//TODO: else "watcher:" if the next thing in sequence is an else_begin,
		//continue the if.  Else, behave exactly as if_end.

		template<typename CurrBuilder>
		auto append(const CurrBuilder &pb, const IfEnd&){
			static_assert(is_IfBuilder<CurrBuilder>::value,
						  "Error: attempt to end If when not in if context! This is a framework bug, please file.");
			return append(pb.prevBuilder, pb.this_if);
		}

		template<typename Cons>
		auto make_if_begin(const Cons &s){
			IfBegin<Cons> r{s};
			return r;
		}

		const IfEnd& make_if_end(){
			static constexpr IfEnd e;
			return e;
		}

	} }
