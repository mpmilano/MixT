#pragma once

#include "typecheck_and_label_decl.hpp"
#include "struct.hpp"

namespace myria {
namespace mtl {
namespace typecheck_phase {

// expressions

template <int seqnum, int depth, typename Obj, typename F, typename Env>
constexpr auto _typecheck(Env, parse_phase::Expression<parse_phase::FieldReference<Obj, F>>)
{
  using checked_struct = DECT(typecheck<seqnum, depth + 1>(Env{}, Obj{}));
  static_assert(is_expression<checked_struct>::value);
  static_assert(mutils::is_struct<typename checked_struct::yield>::value);
  using field_type = DECT(std::declval<typename checked_struct::yield>().field(F{}));
  return Expression<typename checked_struct::label, field_type, FieldReference<checked_struct, F>>{};
}

template <int seqnum, int depth, typename Var, typename Env>
constexpr auto _typecheck(Env, parse_phase::Expression<parse_phase::VarReference<Var>>)
{
  using match = DECT(Env::get_binding(Var{}));
  using label = typename match::label;
  using type = typename match::type;
  return Expression<label, type, VarReference<Var>>{};
}

template <int seqnum, int depth, int i, typename label, typename... Env>
constexpr auto _typecheck(type_environment<label, Env...>, parse_phase::Expression<parse_phase::Constant<i>>)
{
  return Expression<Label<top>, int, Constant<i>>{};
}

template <int seqnum, int depth, char op, typename L, typename R, typename env>
constexpr auto _typecheck(env, parse_phase::Expression<parse_phase::BinOp<op, L, R>>)
{
  using next_l = DECT(typecheck<seqnum, depth + 1>(env{}, L{}));
  using next_r = DECT(typecheck<seqnum + 1, depth + 1>(env{}, R{}));
  using label = resolved_label_min<typename next_l::label, typename next_r::label>;
  using binop = BinOp<op, next_l, next_r>;
  using yield = typename binop::yield;
  return Expression<label, yield, binop>{};
}

// bindings

template <type_location loc, typename label, typename yield, typename Name, typename Expr, typename labele, typename... Env>
constexpr auto enhance_env(type_environment<labele, Env...>, Binding<label, yield, Name, Expr>)
{
  return type_environment<labele, Env..., type_binding<Name, yield, label, loc>>{};
}

// statements
template <int seqnum, int depth, typename Name, typename Expr, typename Body, typename old_env>
constexpr auto _typecheck(old_env, parse_phase::Statement<parse_phase::Let<parse_phase::Binding<Name, Expr>, Body>>)
{
  using binding_expr = DECT(typecheck<seqnum, depth + 1>(old_env{}, Expr{}));
  using next_binding = Binding<Label<temp_label<seqnum, depth>>, typename binding_expr::yield, Name, binding_expr>;
  using new_env = DECT(enhance_env<type_location::local>(old_env{}, next_binding{}));
  using next_body = DECT(typecheck<seqnum + 1, depth + 1>(new_env{}, Body{}));
  return Statement<resolved_label_min<typename binding_expr::label, typename old_env::pc_label>, Let<next_binding, next_body>>{};
}

template <int seqnum, int depth, typename Name, typename Expr, typename Body, typename label_env, typename... Env>
constexpr auto _typecheck(type_environment<label_env, Env...>, parse_phase::Statement<parse_phase::LetRemote<parse_phase::Binding<Name, Expr>, Body>>)
{
  using old_env = type_environment<label_env, Env...>;
  using binding_expr = DECT(typecheck<seqnum, depth + 1>(old_env{}, Expr{}));
  using ptr_label = typename binding_expr::label;
  using handle = typename binding_expr::yield;
  // we dereference the pointer, which is an influencing action.  Reduce the label
  // of the environment if needed.
  using new_env = type_environment<resolved_label_min<label_env, ptr_label>, Env...,
                                   type_binding<Name, handle, resolved_label_min<ptr_label, typename handle::label>, type_location::remote>>;
  using next_body = DECT(typecheck<seqnum + 1, depth + 1>(new_env{}, Body{}));
  using next_binding = Binding<Label<temp_label<seqnum, depth>>, typename handle::type, Name, binding_expr>;
  return Statement<resolved_label_min<label_env, typename handle::label>, LetRemote<next_binding, next_body>>{};
}

	template<typename U, typename V>
	constexpr bool check_type_mismatch(){
		constexpr bool ret = std::is_assignable<U,V>::value || std::is_same<U,V>::value ||
			(std::is_integral<U>::value && std::is_integral<V>::value && std::is_convertible<V,U>::value);
		static_assert(ret);
		return ret;
	}
	
template <int seqnum, int depth, typename Var, typename Expr, typename pc_label, typename... Env>
constexpr auto _typecheck(type_environment<pc_label, Env...>, parse_phase::Statement<parse_phase::Assignment<Var, Expr>>)
{
  using env = type_environment<pc_label, Env...>;
  using new_var = DECT(typecheck<seqnum, depth + 1>(env{}, Var{}));
  using new_expr = DECT(typecheck<seqnum + 1, depth + 1>(env{}, Expr{}));
  static_assert(check_type_mismatch<typename new_var::yield, typename new_expr::yield>(), "Error: type mismatch in assignment");
  return Statement<resolved_label_min<resolved_label_min<typename new_expr::label, typename new_var::label>, pc_label>, Assignment<new_var, new_expr>>{};
}

template <int seqnum, int depth, typename condition, typename body, typename pc_label, typename... Env>
constexpr auto _typecheck(type_environment<pc_label, Env...>, parse_phase::Statement<parse_phase::While<condition, body>>)
{
  using old_env = type_environment<pc_label, Env...>;
  using next_condition = DECT(typecheck<seqnum, depth + 1>(old_env{}, condition{}));
  static_assert(std::is_same<typename next_condition::yield, bool>::value, "Error: condition for while loop must be bool");
  using new_label = resolved_label_min<typename next_condition::label, pc_label>;
  using new_env = type_environment<new_label, Env...>;
  using next_body = DECT(typecheck<seqnum + 1, depth + 1>(new_env{}, body{}));
  return Statement<resolved_label_min<pc_label, typename next_condition::label>, While<next_condition, next_body>>{};
}

template <int seqnum, int depth, typename condition, typename then, typename els, typename pc_label, typename... Env>
constexpr auto _typecheck(type_environment<pc_label, Env...>, parse_phase::Statement<parse_phase::If<condition, then, els>>)
{
  using old_env = type_environment<pc_label, Env...>;
  using next_condition = DECT(typecheck<seqnum, depth + 1>(old_env{}, condition{}));
  using new_label = resolved_label_min<typename next_condition::label, pc_label>;
  using new_env = type_environment<new_label, Env...>;
  using next_then = DECT(typecheck<seqnum + 1, depth + 1>(new_env{}, then{}));
  using next_else = DECT(typecheck<seqnum + 2, depth + 1>(new_env{}, els{}));
  return Statement<resolved_label_min<pc_label, typename next_condition::label>, If<next_condition, next_then, next_else>>{};
}

template <int seqnum, int depth, typename Env>
constexpr auto _typecheck(parse_phase::Sequence<> )
{
  return Sequence<>{};
}

template <int seqnum, int depth, typename Env, typename stmt, typename... Statements>
constexpr auto _typecheck(parse_phase::Sequence<stmt, Statements...>)
{
  return Sequence<DECT(typecheck<seqnum, depth>(Env{}, stmt{}))>::append(_typecheck<seqnum + 1, depth, Env>(parse_phase::Sequence<Statements...>{}));
}

template <int seqnum, int depth, typename Env, typename... Statements>
constexpr auto _typecheck(Env, parse_phase::Statement<parse_phase::Sequence<Statements...>>)
{
  using label = typename Env::pc_label;
  return Statement<label, DECT(_typecheck<seqnum, depth, Env>(parse_phase::Sequence<Statements...>{}))>{};
}

// entry points

template <int seqnum, int depth, typename T, typename pc_label, typename... Bindings>
constexpr auto typecheck(type_environment<pc_label, Bindings...>, parse_phase::Statement<T>)
{
  using checked = DECT(_typecheck<seqnum, depth>(type_environment<pc_label, Bindings...>{}, parse_phase::Statement<T>{}));
  return checked{};
}
template <int seqnum, int depth, typename T, typename pc_label, typename... Bindings>
constexpr auto typecheck(type_environment<pc_label, Bindings...>, parse_phase::Expression<T>)
{
  using checked = DECT(_typecheck<seqnum, depth>(type_environment<pc_label, Bindings...>{}, parse_phase::Expression<T>{}));
  return checked{};
}
}
}
}
