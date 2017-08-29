#pragma once
#include "pre_endorse.hpp"
#include "worklist.hpp"

namespace myria {
namespace mtl {
namespace typecheck_phase {

	template<typename endorse_variable_list, typename AST> constexpr auto endorse_relabel(AST);

//set the "endorse_variable_list" field whenever the expression (or binding) needs to be endorsed.
	
template <typename endorse_variable_list, typename l, typename y, typename v, typename e>
constexpr auto _endorse_relabel(const Binding<l, y, v, Expression<l,y,e> >&)
{
	using new_e = DECT(endorse_relabel<endorse_variable_list>(Expression<l,y,e>{}));
	if constexpr (endorse_variable_list::template contains<v>()){
			return Binding<Label<PreEndorse_notop<l> >, y,v,new_e >{};
		}
	else return Binding<l, y,v,new_e >{};
}

template <typename endorse_variable_list, typename l, typename y, typename s, typename f>
constexpr auto _endorse_relabel(const Expression<l, y, FieldReference<s, f>>&)
{
	using sub = DECT(endorse_relabel<endorse_variable_list>(s{}));
	return Expression<typename sub::label, y, FieldReference<sub,f> >{};
}

template <typename endorse_variable_list, typename l, typename y, typename v>
constexpr auto _endorse_relabel(const Expression<l, y, VarReference<v>>&)
{
	if constexpr (endorse_variable_list::template contains<v>()) {
			return Expression<Label<PreEndorse_notop<l>>,y,VarReference<v> >{};
		}
	else {
		return Expression<l,y,VarReference<v> >{};
	}
}

template <typename endorse_variable_list, int i>
constexpr auto _endorse_relabel(const Expression<Label<top>, int, Constant<i>>& a)
{
	return a;
}

template <typename endorse_variable_list, typename l, typename y, char op, typename L, typename R>
constexpr auto _endorse_relabel(const Expression<l, y, BinOp<op, L, R>>&)
{
	using subL = DECT(endorse_relabel<endorse_variable_list>(L{}));
	using subR = DECT(endorse_relabel<endorse_variable_list>(R{}));
	return Expression<resolved_label_min<typename subL::label, typename subR::label>, y, BinOp<op,subL,subR> >{};
}

template <typename endorse_variable_list, typename l, typename y, typename h>
constexpr auto _endorse_relabel(const Expression<l, y, IsValid<h>>&)
{
	using sub = DECT(endorse_relabel<endorse_variable_list>(h{}));
	return Expression<typename sub::label,y, IsValid<sub> >{};
}

	template <typename endorse_variable_list, typename l, typename y, typename h>
constexpr auto _endorse_relabel(const Expression<l, y, Endorse<l,h>>)
{
	//we have already dispatched this endorsement; we no longer need the node.
	return endorse_relabel<endorse_variable_list>(h{});
}


template <typename endorse_variable_list, typename l, typename b, typename body>
constexpr auto _endorse_relabel(const Statement<l, Let<b, body>>&)
{
	using newb = DECT(endorse_relabel<endorse_variable_list>(b{}));
	using newbod = DECT(endorse_relabel<endorse_variable_list>(body{}));
	return Statement<typename newb::label, Let<newb,newbod> >{};
}

template <typename endorse_variable_list, typename l, typename b, typename body>
constexpr auto _endorse_relabel(const Statement<l, LetRemote<b, body>>&)
{
	using newb = DECT(endorse_relabel<endorse_variable_list>(b{}));
	using newbod = DECT(endorse_relabel<endorse_variable_list>(body{}));
	using newlr = LetRemote<newb, newbod>;
	if constexpr (is_pre_endorsed<typename newb::label>()){
			return Statement<Label<PreEndorse_notop<l>>, newlr>{};
		}
	else return Statement<l,newlr>{};
}


template <typename endorse_variable_list, typename oper_name, typename Hndl, typename... args>
constexpr auto _endorse_relabel(Operation<oper_name,Hndl,args...> a)
{
	using newh = DECT(endorse_relabel<endorse_variable_list>(Hndl{}));
	static_assert(!(is_pre_endorsed<typename newh::label>() || ... || is_pre_endorsed<typename DECT(endorse_relabel<endorse_variable_list>(args{}))::label>()),
		"Error: cannot run an operation in pre-endorse step");
	return a;
}
template <typename endorse_variable_list, typename l, typename y, typename oper_name, typename Hndl, typename... args>
constexpr auto _endorse_relabel(const Expression<l, y, Operation<oper_name,Hndl,args...>>& )
{
	return Expression<l, y, DECT(_endorse_relabel<endorse_variable_list>(Operation<oper_name,Hndl,args...>{}))>{};
}
template <typename endorse_variable_list, typename l, typename oper_name, typename Hndl, typename... args>
constexpr auto _endorse_relabel(const Statement<l, Operation<oper_name,Hndl,args...>>&)
{
	return Statement<l,DECT(_endorse_relabel<endorse_variable_list>(Operation<oper_name,Hndl,args...>{}))>{};
}

	
template <typename endorse_variable_list, typename l, typename L, typename R>
constexpr auto _endorse_relabel(const Statement<l, Assignment<L, R>>& a)
{
	static_assert(!is_pre_endorsed<typename DECT(endorse_relabel<endorse_variable_list>(L{}))::label>() || 
				  !is_pre_endorsed<typename DECT(endorse_relabel<endorse_variable_list>(R{}))::label>(),
				  "Error: cannot assign during pre-endorse step");
	return a;
}

template <typename endorse_variable_list, typename l, typename R>
constexpr auto _endorse_relabel(const Statement<l, Return<R> >&)
{
	using sub = DECT(endorse_relabel<endorse_variable_list>(R{}));
	return Statement<Label<bottom>, Return<sub> >{};
}


template <typename endorse_variable_list, typename l, typename c, typename t, typename e>
constexpr auto _endorse_relabel(const Statement<l, If<c, t, e>>&)
{
	using newc = DECT(endorse_relabel<endorse_variable_list>(c{}));
	using newt = DECT(endorse_relabel<endorse_variable_list>(t{}));
	using newe = DECT(endorse_relabel<endorse_variable_list>(e{}));
	using newif = If<newc,newt,newe>;
	if constexpr (is_pre_endorsed<typename newc::label>()){
			return Statement<Label<PreEndorse_notop<l>>, newif>{};
		}
	else return Statement<l,newif>{};
}

template <typename endorse_variable_list, typename l, typename c, typename t, char... name>
constexpr auto _endorse_relabel(const Statement<l, While<Expression<l,bool,VarReference<c> >, t, name...>>&)
{
	using newc = DECT(endorse_relabel<endorse_variable_list>(Expression<l,bool,VarReference<c> >{}));
	using newt = DECT(endorse_relabel<endorse_variable_list>(t{}));
	using new_while = While<newc,newt,name...>;
	return Statement<typename newc::label, new_while>{};
}

template <typename endorse_variable_list, typename l, typename... Seq>
constexpr auto _endorse_relabel(const Statement<l, Sequence<Seq...>>&)
{
	return Statement<l,Sequence<DECT(endorse_relabel<endorse_variable_list>(Seq{}))...> >{};
}

	template<typename endorse_variable_list, typename AST> constexpr auto endorse_relabel(AST a){
		return _endorse_relabel<endorse_variable_list>(a);
	}


	template<bool b, typename WL>
	struct contains_endorsement_argument : public std::integral_constant<bool,b>{};

	template<bool already_found, typename worklist, typename AST>
	constexpr auto contains_endorsement(contains_endorsement_argument<already_found,worklist>, AST);
	
	template<bool already_found, typename worklist, typename l, typename lold, typename v, typename y, typename T>
	constexpr auto contains_endorsement(contains_endorsement_argument<already_found,worklist>,
										Expression<l, y, Endorse<l,Expression<lold,y,VarReference<v> > > >){
		return contains_endorsement_argument<true,worklist>{};
	}
	template<bool already_found, typename worklist, typename l, typename lold, typename v, typename y, typename T>
	constexpr auto contains_endorsement(contains_endorsement_argument<already_found,worklist>,
										Expression<l, y, VarReference<v> >){
		return worklist::current_elements::template contains<v>();
	}

	template<typename Arg, typename AST>
	using Contains_endorsement = DECT(contains_endorsement(Arg{},AST{}));

	template<bool already_found, typename worklist, typename AST>
	constexpr auto contains_endorsement(contains_endorsement_argument<already_found,worklist>, AST){
		constexpr bool b = already_found || AST::template fold<Contains_endorsement,already_found>::value;
		return contains_endorsement_argument<b,worklist>{};
	}
	
//set the "current_worklist" field whenever the expression (or binding) needs to be endorsed.

	template<typename current_worklist, typename AST>
	constexpr auto add_all_varrefs(AST);

	template<typename current_worklist, typename l, typename y, typename v>
	constexpr auto add_all_varrefs(Expression<l,y,VarReference<v> >){
		return current_worklist::template append<v>();
	}
	
	template<typename current_worklist, typename AST>
	using Add_all_varrefs = DECT(add_all_varrefs<current_worklist>(AST{}));
	
	template<typename current_worklist, typename AST>
	constexpr auto add_all_varrefs(AST){
		return typename AST::template default_recurse<Add_all_varrefs, current_worklist, mutils::Combine_worklists, current_worklist>{};
	}

	template<typename current_worklist, typename AST> constexpr auto collect_pre_endorse(AST);
	
	template <typename current_worklist, typename l, typename lold, typename y, typename v>
	constexpr auto _collect_pre_endorse(const Expression<l, y, Endorse<l,Expression<lold,y,VarReference<v> > > >&)
	{
		return current_worklist::template append<v>();
	}

	template <typename current_worklist, typename l, typename y, typename v, typename e>
	constexpr auto _collect_pre_endorse(const Binding<l, y, v, e >&){
		if constexpr (current_worklist::current_elements::template contains<v>()){
				return add_all_varrefs<current_worklist>(e{});
			}
		else return collect_pre_endorse<current_worklist>(e{});
	}

template <typename current_worklist, typename l, typename v, typename t, typename e>
constexpr auto _collect_pre_endorse(const Statement<l, If<Expression<l,bool,VarReference<v> >, t, e> >&)
{
	constexpr auto partial = collect_pre_endorse<current_worklist>(t{}).combine(collect_pre_endorse<current_worklist>(e{}));
	using c = Expression<l,bool,VarReference<v> >;
	if constexpr (Contains_endorsement<contains_endorsement_argument<false,current_worklist>, If<c, t, e> >::value){
			return partial.template append<v>;
		}
	else return partial;
}

template <typename current_worklist, typename l, typename v, typename t, char... name>
constexpr auto _collect_pre_endorse(const Statement<l, While<Expression<l,bool,VarReference<v> >, t, name...>>&)
{
	constexpr auto partial = collect_pre_endorse<current_worklist>(t{});
	if constexpr (Contains_endorsement<contains_endorsement_argument<false,current_worklist>, t >::value){
			return partial.template append<v>;
		}
	else return partial;
}

	//boilerplate / defaults
	template<typename current_worklist, typename AST>
	using Collect_pre_endorse = DECT(collect_pre_endorse<current_worklist>(AST{}));
	
	template <typename current_worklist, typename AST>
	constexpr auto _collect_pre_endorse(const AST&){
		return typename AST::template default_recurse<Collect_pre_endorse, current_worklist, mutils::Combine_worklists, current_worklist>{};
	}
	
	template<typename current_worklist, typename AST> constexpr auto collect_pre_endorse(AST a){
		return _collect_pre_endorse<current_worklist>(a);
	}
	//end boiler

	template<typename AST, typename... ts>
	constexpr auto do_pre_endorse(mutils::WorkList<mutils::typelist<>, mutils::typeset<ts...> > ){
		return endorse_relabel<mutils::typelist<ts...> >(AST{});
	}

	template<typename AST, typename ts, typename fst, typename... rst>
	constexpr auto do_pre_endorse(mutils::WorkList<mutils::typelist<fst,rst...>, ts> ){
		return do_pre_endorse<AST>(collect_pre_endorse<mutils::WorkList<mutils::typelist<rst...>, ts> > (AST{}));
	}

	template<typename AST>
	constexpr auto do_pre_endorse(AST){
		return do_pre_endorse<AST>(mutils::WorkList<mutils::typelist<mutils::mismatch,mutils::mismatch>, mutils::typeset<> >{});
	}

	
}
}
}
