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
	return Binding<typename new_e::label, y,v,new_e >{};
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
			return Expression<Label<PreEndorse<l>>,y,VarReference<v> >{};
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
			return Statement<Label<PreEndorse<l>>, newlr>{};
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
	return Statement<typename sub::label, Return<sub> >{};
}


template <typename endorse_variable_list, typename l, typename c, typename t, typename e>
constexpr auto _endorse_relabel(const Statement<l, If<c, t, e>>&)
{
	using newc = DECT(endorse_relabel<endorse_variable_list>(c{}));
	using newt = DECT(endorse_relabel<endorse_variable_list>(t{}));
	using newe = DECT(endorse_relabel<endorse_variable_list>(e{}));
	using newif = If<newc,newt,newe>;
	if constexpr (is_pre_endorsed<typename newc::label>()){
			return Statement<Label<PreEndorse<l>>, newif>{};
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

//set the "current_worklist" field whenever the expression (or binding) needs to be endorsed.

//let's say that the expression-ones return booleans that indicate whether the relevant items are present,
//whereas the statement-ones return the new worklist.  The worklist's "ever-seen" set is then the final
//list of variables

	template<typename current_worklist, typename AST> constexpr auto collect_pre_endorse(AST);
	
template <typename current_worklist, typename l, typename y, typename v, typename e>
constexpr auto _collect_pre_endorse(const Binding<l, y, v, Expression<l,y,e> >&)
{
	if constexpr (collect_pre_endorse<current_worklist>(Expression<l,y,e>{})){
			return current_worklist::template append<v>();
		}
	else return current_worklist{};
}

template <typename current_worklist, typename l, typename y, typename s, typename f>
constexpr auto _collect_pre_endorse(const Expression<l, y, FieldReference<s, f>>&)
{
	return collect_pre_endorse<current_worklist>(s{});
}

template <typename current_worklist, typename l, typename y, typename v>
constexpr auto _collect_pre_endorse(const Expression<l, y, VarReference<v>>&)
{
	return current_worklist::current_elements::template contains<v>();
}

template <typename current_worklist, int i>
constexpr auto _collect_pre_endorse(const Expression<Label<top>, int, Constant<i>>& )
{
	return false;
}

template <typename current_worklist, typename l, typename y, char op, typename L, typename R>
constexpr auto _collect_pre_endorse(const Expression<l, y, BinOp<op, L, R>>&)
{
	return collect_pre_endorse<current_worklist>(L{}) || collect_pre_endorse<current_worklist>(R{});
}

template <typename current_worklist, typename l, typename y, typename h>
constexpr auto _collect_pre_endorse(const Expression<l, y, IsValid<h>>&)
{
	return collect_pre_endorse<current_worklist>(h{});
}

	template <typename current_worklist, typename l, typename name, typename lold, typename y, typename v, typename body>
	constexpr auto _collect_pre_endorse(const Statement<l, Let<Binding<l,y,name, Expression<l, y, Endorse<l,Expression<lold,y,VarReference<v> > > > >, body > > )
{	
	return current_worklist::template append<v>().combine(collect_pre_endorse<current_worklist>(body{}));
}
	
	template <typename current_worklist, typename l, typename name, typename lold, typename y, typename v, typename body>
	constexpr auto _collect_pre_endorse(const Statement<l, LetRemote<Binding<l,y, name,Expression<l, y, Endorse<l,Expression<lold,y,VarReference<v> > > > >, body > > )
{	
	return current_worklist::template append<v>().combine(collect_pre_endorse<current_worklist>(body{}));
}

template <typename current_worklist, typename l, typename b, typename body>
constexpr auto _collect_pre_endorse(const Statement<l, Let<b, body>>&)
{
	return collect_pre_endorse<current_worklist>(b{}).combine(collect_pre_endorse<current_worklist>(body{}));
}

template <typename current_worklist, typename l, typename b, typename body>
constexpr auto _collect_pre_endorse(const Statement<l, LetRemote<b, body>>&)
{
	return collect_pre_endorse<current_worklist>(b{}).combine(collect_pre_endorse<current_worklist>(body{}));
}
	
template <typename current_worklist, typename l, typename y, typename oper_name, typename Hndl, typename... args>
constexpr auto _collect_pre_endorse(const Expression<l, y, Operation<oper_name,Hndl,args...>>& )
{
	return (collect_pre_endorse<current_worklist>(Hndl{}) || ... || collect_pre_endorse<current_worklist>(args{}));
}
template <typename current_worklist, typename l, typename oper_name, typename Hndl, typename... args>
constexpr auto _collect_pre_endorse(const Statement<l, Operation<oper_name,Hndl,args...>>&)
{
	return current_worklist{};
}
	
template <typename current_worklist, typename l, typename L, typename R>
constexpr auto _collect_pre_endorse(const Statement<l, Assignment<L, R>>& )
{
	//no need to convey influence here, since it will be an error to have an endorser in assignment.
	//same with operations above.
	return current_worklist{};
}

template <typename current_worklist, typename l, typename R>
constexpr auto _collect_pre_endorse(const Statement<l, Return<R> >&)
{
	return current_worklist{};
}

	template<bool b, typename WL>
	struct contains_endorsement_argument : public std::integral_constant<bool,b>{};

	template<bool already_found, typename worklist, typename l, typename T>
	constexpr auto contains_endorsement(contains_endorsement_argument<already_found,worklist>, Statement<l,T>);
	template<bool already_found, typename worklist, typename l, typename y, typename T>
	constexpr auto contains_endorsement(contains_endorsement_argument<already_found,worklist>, Expression<l,y,T> ce);

	template<bool already_found, typename worklist, typename l, typename lold, typename v, typename y, typename T>
	constexpr auto contains_endorsement(contains_endorsement_argument<already_found,worklist>,
										Expression<l, y, Endorse<l,Expression<lold,y,VarReference<v> > > >){
		return contains_endorsement_argument<true,worklist>{};
	}

	template<typename Arg, typename AST>
	using Contains_endorsement = DECT(contains_endorsement(Arg{},AST{}));

	template<bool already_found, typename worklist, typename l, typename T>
	constexpr auto contains_endorsement(contains_endorsement_argument<already_found,worklist>, Statement<l,T>){
		constexpr bool b = already_found || Statement<l,T>::template fold<Contains_endorsement,already_found>::value;
		return contains_endorsement_argument<b,worklist>{};
	}
	template<bool already_found, typename worklist, typename l, typename y, typename T>
	constexpr auto contains_endorsement(contains_endorsement_argument<already_found,worklist>, Expression<l,y,T> ce){
		constexpr bool b = already_found || collect_pre_endorse<worklist>(ce);
		return contains_endorsement_argument<b,worklist>{};
	}

	template<typename current_worklist, typename AST>
	using Collect_pre_endorse = DECT(collect_pre_endorse<current_worklist>(AST{}));

	template<typename wl1, typename wl2> using Combine_worklists = DECT(wl1::combine(wl2{}));

	template<typename current_worklist, typename AST>
	using pre_endorse_default = AST::default_recurse<Collect_pre_endorse, current_worklist, Combine_worklists, current_worklist>
	
template <typename current_worklist, typename l, typename s>
constexpr auto _collect_pre_endorse(const Statement<l,s>){
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

template <typename current_worklist, typename l, typename... Seq>
constexpr auto _collect_pre_endorse(const Statement<l, Sequence<Seq...>>&)
{
	return current_worklist::combine(collect_pre_endorse<current_worklist>(Seq{})...);
}
	
	template<typename current_worklist, typename AST> constexpr auto collect_pre_endorse(AST a){
		return _collect_pre_endorse<current_worklist>(a);
	}

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
