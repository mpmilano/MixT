#pragma once

namespace myria {
namespace mtl {
namespace typecheck_phase {

	template<typename endorse_variable_list, typename AST> auto endorse_relabel(AST);

//set the "endorse_variable_list" field whenever the expression (or binding) needs to be endorsed.
	
template <typename endorse_variable_list, typename l, typename y, typename v, typename e>
auto _endorse_relabel(const Binding<l, y, v, Expression<l,y,e> >&)
{
	using new_e = endorse_relabel<endorse_variable_list>(Expression<l,y,e>{});
	if constexpr (endorse_variable_list::contains<v>()){
			return Binding<PreEndorse<l>, y,v,new_e >{};
		}
	else return Binding<typename new_e::label, y,v,new_e >{};
	
	static_assert(false); (void) endorse_variable_list;
}

template <typename endorse_variable_list, typename l, typename y, typename s, typename f>
auto _endorse_relabel(const Expression<l, y, FieldReference<s, f>>&)
{
	using sub = DECT(endorse_relabel<endorse_variable_list>(s{}));
	return Expression<typename sub::label, y, FieldReference<sub,f> >{};
}

template <typename endorse_variable_list, typename l, typename y, typename v>
auto _endorse_relabel(const Expression<l, y, VarReference<v>>&)
{
	if constexpr (endorse_variable_list::contains<v>()) {
			return Expression<PreEndorse<l>,y,VarReference<v> >{};
		}
	else {
		return Expression<l,y,VarReference<v> >{};
	}
}

template <typename endorse_variable_list, int i>
auto _endorse_relabel(const Expression<Label<top>, int, Constant<i>>& a)
{
	return a;
}

template <typename endorse_variable_list, typename l, typename y, char op, typename L, typename R>
auto _endorse_relabel(const Expression<l, y, BinOp<op, L, R>>&)
{
	using subL = DECT(endorse_relabel<endorse_variable_list>(L{}));
	using subR = DECT(endorse_relabel<endorse_variable_list>(R{}));
	return Expression<resolved_label_min<typename subL::label, typename subR::label>, y, BinOp<op,subL,subR> >{};
}

template <typename endorse_variable_list, typename l, typename y, typename h>
auto _endorse_relabel(const Expression<l, y, IsValid<h>>&)
{
	using sub = DECT(endorse_relabel<endorse_variable_list>(h{}));
	return Expression<typename sub::label,y, IsValid<sub> >{};
}

	template <typename endorse_variable_list, typename l, typename lold, typename y, typename h>
	auto _endorse_relabel(const Expression<l, y, Endorse<lold,h>> a)
{
	return Expression<l, y, Endorse<PreEndorse<lold>,h> >{};
}


template <typename endorse_variable_list, typename l, typename b, typename body>
auto _endorse_relabel(const Statement<l, Let<b, body>>&)
{
	using newb = DECT(endorse_relabel<endorse_variable_list>(b{}));
	using newbod = DECT(endorse_relabel<endorse_variable_list>(body{}));
	return Statement<typename newb::label, Let<newb,newbod> >{};
}

template <typename endorse_variable_list, typename l, typename b, typename body>
auto _endorse_relabel(const Statement<l, LetRemote<b, body>>&)
{
	using newb = DECT(endorse_relabel<endorse_variable_list>(b{}));
	using newbod = DECT(endorse_relabel<endorse_variable_list>(body{}));
	using newlr = LetRemote<newb, newbod>;
	if constexpr (is_pre_endorsed<typename newb::label>()){
			return Statement<PreEndorse<l>, newlr>{};
		}
	else return Statement<l,newlr>{};
}


template <typename endorse_variable_list, typename oper_name, typename Hndl, typename... args>
auto _endorse_relabel(Operation<oper_name,Hndl,args...> a)
{
	using newh = DECT(endorse_relabel<endorse_variable_list>(Hndl{}));
	static_assert(!is_pre_endorsed<typename newh::label>() || ... || !is_pre_endorsed<typename DECT(endorse_relabel<endorse_variable_list>(args{}))::label>(),
		"Error: cannot run an operation in pre-endorse step");
	return a;
}
template <typename endorse_variable_list, typename l, typename y, typename oper_name, typename Hndl, typename... args>
auto _endorse_relabel(const Expression<l, y, Operation<oper_name,Hndl,args...>>& a)
{
	return Expression<l, y, DECT(_endorse_relabel<endorse_variable_list>(Operation<oper_name,Hndl,args...>{}))>{};
}
template <typename endorse_variable_list, typename l, typename oper_name, typename Hndl, typename... args>
auto _endorse_relabel(const Statement<l, Operation<oper_name,Hndl,args...>>&)
{
	return Statement<l,DECT(_endorse_relabel<endorse_variable_list>(Operation<oper_name,Hndl,args...>{}))>{};
}

	
template <typename endorse_variable_list, typename l, typename L, typename R>
auto _endorse_relabel(const Statement<l, Assignment<L, R>>& a)
{
	static_assert(!is_pre_endorsed<typename DECT(endorse_relabel<endorse_variable_list>(L{}))::label>() || 
				  !is_pre_endorsed<typename DECT(endorse_relabel<endorse_variable_list>(R{}))::label>(),
				  "Error: cannot assign during pre-endorse step");
	return a;
}

template <typename endorse_variable_list, typename l, typename R>
auto _endorse_relabel(const Statement<l, Return<R> >&)
{
	using sub = DECT(endorse_relabel<endorse_variable_list>(R{}));
	return Statement<typename sub::label, Return<sub> >{};
}


template <typename endorse_variable_list, typename l, typename c, typename t, typename e>
auto _endorse_relabel(const Statement<l, If<c, t, e>>&)
{
	using newc = DECT(endorse_relabel<endorse_variable_list>(c{}));
	using newt = DECT(endorse_relabel<endorse_variable_list>(t{}));
	using newe = DECT(endorse_relabel<endorse_variable_list>(e{}));
	using newif = If<newc,newt,newe>;
	if constexpr (is_pre_endorsed<typename newc::label>()){
			return Statement<PreEndorse<l>, newif>{};
		}
	else return Statement<l,newif>{};
}

template <typename endorse_variable_list, typename l, typename c, typename t, char... name>
auto _endorse_relabel(const Statement<l, While<Expression<l,bool,VarReference<c> >, t, name...>>&)
{
	using newc = DECT(endorse_relabel<endorse_variable_list>(Expression<l,bool,VarReference<c> >{}));
	using newt = DECT(endorse_relabel<endorse_variable_list>(t{}));
	using new_while = While<newc,newt,name...>;
	return Statement<typename newc::label, new_while>{};
}

template <typename endorse_variable_list, typename l, typename... Seq>
auto _endorse_relabel(const Statement<l, Sequence<Seq...>>&)
{
	return Statement<l,DECT(endorse_relabel<endorse_variable_list>(Seq))...>{};
}

	template<typename endorse_variable_list, typename AST> auto endorse_relabel(AST a){
		return _endorse_relabel<endorse_variable_list>(a);
	}

//set the "current_worklist" field whenever the expression (or binding) needs to be endorsed.

//let's say that the expression-ones return booleans that indicate whether the relevant items are present,
//whereas the statement-ones return the new worklist.  The worklist's "ever-seen" set is then the final
//list of variables

	template<typename current_worklist, typename AST> auto collect_pre_endorse(AST);
	
template <typename current_worklist, typename l, typename y, typename v, typename e>
constexpr auto _collect_pre_endorse(const Binding<l, y, v, Expression<l,y,e> >&)
{
	if constexpr (collect_pre_endorse<current_worklist>(Expression<l,y,e>{})){
			return current_worklist::append<v>();
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
	return current_worklist::current_elements::contains<v>();
}

template <typename current_worklist, int i>
constexpr auto _collect_pre_endorse(const Expression<Label<top>, int, Constant<i>>& a)
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

template <typename current_worklist, typename l, typename lold, typename y, typename v>
constexpr auto _collect_pre_endorse(const Expression<l, y, Endorse<l,Expression<lold,y,VarReference<v> > > > a)
{	
	return current_worklist::add<v>();
}


template <typename current_worklist, typename l, typename b, typename body>
constexpr auto _collect_pre_endorse(const Statement<l, Let<b, body>>&)
{
	return collect_pre_endorse<current_worklist>(b{})::combine(collect_pre_endorse<current_worklist>(body{}));
}

template <typename current_worklist, typename l, typename b, typename body>
constexpr auto _collect_pre_endorse(const Statement<l, LetRemote<b, body>>&)
{
	return collect_pre_endorse<current_worklist>(b{})::combine(collect_pre_endorse<current_worklist>(body{}));
}
	
template <typename current_worklist, typename l, typename y, typename oper_name, typename Hndl, typename... args>
constexpr auto _collect_pre_endorse(const Expression<l, y, Operation<oper_name,Hndl,args...>>& a)
{
	return (collect_pre_endorse<current_worklist>(Hndl{}) || ... || collect_pre_endorse<current_worklist>(args{}));
}
template <typename current_worklist, typename l, typename oper_name, typename Hndl, typename... args>
constexpr auto _collect_pre_endorse(const Statement<l, Operation<oper_name,Hndl,args...>>&)
{
	return current_worklist{};
}
	
template <typename current_worklist, typename l, typename L, typename R>
constexpr auto _collect_pre_endorse(const Statement<l, Assignment<L, R>>& a)
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


template <typename current_worklist, typename l, typename c, typename t, typename e>
constexpr auto _collect_pre_endorse(const Statement<l, If<c, t, e> >&)
{
	static_assert(false,"if there is an endorsement anywhere in t or e, need to upgrade c.");
	return collect_pre_endorse<current_worklist>(t{}).combine(collect_pre_endorse<current_worklist>(e{}));
}

template <typename current_worklist, typename l, typename c, typename t, char... name>
constexpr auto _collect_pre_endorse(const Statement<l, While<c, t, name...>>&)
{
	static_assert(false,"if there is an endorsement anywhere in t, need to upgrade c.");
	return collect_pre_endorse<current_worklist>(t{});
}

template <typename current_worklist, typename l, typename... Seq>
constexpr auto _collect_pre_endorse(const Statement<l, Sequence<Seq...>>&)
{
	return current_worklist::combine(collect_pre_endorse<current_worklist>(Seq{})...);
}
	
	template<typename current_worklist, typename AST> auto collect_pre_endorse(AST a){
		return _collect_pre_endorse<current_worklist>(a);
	}

	template<typename AST, typename... ts>
	auto do_pre_endorse(mutils::WorkList<mutils::typelist<>, mutils::typeset<ts...> > wl){
		return endorse_relabel<mutils::typelist<ts...> >(AST{});
	}

	template<typename AST, typename ts, typename fst, typename... rst>
	auto do_pre_endorse(mutils::WorkList<mutils::typelist<fst,rst...>, ts> ){
		return do_pre_endorse<AST>(collect_pre_endorse<mutils::WorkList<mutils::typelist<rst...>, ts> > (AST{}));
	}

	template<typename AST>
	auto do_pre_endorse(AST a){
		return do_pre_endorse<AST>(mutils::WorkList<mutils::typelist<mutils::mismatch,mutils::mismatch>, mutils::typeset<> >{});
	}

	
}
}
}
