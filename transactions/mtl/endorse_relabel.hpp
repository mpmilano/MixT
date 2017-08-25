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
		return _endorse_relabel<endorse_variable_list>(endorse_variable_list, a);
	}
	
}
}
}
