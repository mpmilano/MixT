#pragma once

#include "AST_typecheck.hpp"
#include "AST_split.hpp"
#include "mtlutils.hpp"
#include "name_while.hpp"
#include "runnable_transaction.hpp"
#include "remove_unused.hpp"
#include "without_names.hpp"
#include "remove_empties.hpp"

namespace myria {
namespace mtl {

namespace typecheck_phase {
template <typename>
struct base_var_str;
template <typename l, typename y, char... c>
struct base_var_str<Expression<l, y, VarReference<String<c...>>>>
{
  using type = String<c...>;
};
template <typename l, typename y, typename F, typename S>
struct base_var_str<Expression<l, y, FieldReference<F, S>>> : public base_var_str<F>
{
};
template <typename T>
using base_var = typename base_var_str<T>::type;
}

namespace split_phase {

template <typename _label, typename provides, typename inherits, typename _ast, typename requires>
struct extracted_phase<Label<_label>, phase_api<Label<_label>, requires, provides, inherits>, _ast>
{
  using label = Label<_label>;
  using api = phase_api<Label<_label>, requires, provides, inherits>;
  using ast = _ast;
  static_assert(AST<label>::template is_ast<ast>::value);
};

template <typename _label, typename provides, typename inherits, typename _ast, typename requires>
static constexpr auto transaction_prephase(extracted_phase<Label<_label>, phase_api<Label<_label>, requires, provides, inherits>, _ast> a)
{
  using This = DECT(a);
  return runnable_transaction::prephase<typename This::label, typename This::ast, DECT(binding_to_holder(to_typeset(typename provides::as_typelist{}))),
                                        DECT(binding_to_holder(to_typeset(typename inherits::as_typelist{}))),
                                        DECT(binding_to_holder(to_typeset(typename requires::as_typelist{})))>{};
}

template <typename l>
template <typename Yields, typename label2, typename Str, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Expression<label2, Yields, typecheck_phase::VarReference<Str>>)
{
  using needed_binding = type_binding_super<Str, Yields, label2>;
  static_assert(label2::flows_to(label{}), "Error: encountered VarReference before passing definition phase");
  static_assert(old_api::provides::template contains_subtype<needed_binding>() || old_api::inherits::template contains_subtype<needed_binding>());
  using requires = std::conditional_t<old_api::provides::template contains_subtype<needed_binding>(), requires<>,
                                      requires<typename old_api::inherits::template find_subtype<needed_binding>>>;
  return extracted_phase<label, phase_api<label, requires, provides<>, typename old_api::inherits>, Expression<Yields, VarReference<Str>>>{};
}

template <typename label, typename old_api, typename label2, typename Yields, typename var, typename expr>
constexpr auto let_binding(old_api, typecheck_phase::Binding<label2, Yields, var, expr>)
{
  // we can descend into a mismatched binding, but only when that binding is
  // used *later* than us.
  static_assert(label::flows_to(label2{}));
  using new_binding = type_binding<var, Yields, label2, type_location::local>;
  using processed_expr = DECT(AST<label>::collect_phase(old_api{}, expr{}));
  using returned_api = DECT(processed_expr::api::add_provides(new_binding{}));
  return extracted_phase<label, returned_api, typename AST<label>::template Binding<label2, Yields, var, typename processed_expr::ast>>{};
}

template <typename label, typename label2, typename Yields, typename var, typename exprl, typename handle_t, typename expr, typename phase_api>
constexpr auto let_remote_binding(phase_api, typecheck_phase::Binding<label2, Yields, var, typecheck_phase::Expression<exprl, handle_t, expr>>)
{
  // we can descend into a mismatched binding, but only when that binding is
  // used *later* than us.
  static_assert(label::flows_to(label2{}));
  using new_binding = type_binding<var, handle_t, label2, type_location::remote>;
  using processed_expr = DECT(AST<label>::collect_phase(phase_api{}, typecheck_phase::Expression<exprl, handle_t, expr>{}));
  using returned_api = DECT(processed_expr::api::add_provides(new_binding{}));
  return extracted_phase<label, returned_api, typename AST<label>::template Binding<label2, Yields, var, typename processed_expr::ast>>{};
}

template <typename l>
template <typename _binding, typename Body, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label, typecheck_phase::Let<_binding, Body>>)
{
  using binding = DECT(let_binding<label>(old_api{}, _binding{}));
  using binding_ast = typename binding::ast;
  using body = DECT(collect_phase(combined_api<typename binding::api, old_api>{}, Body{}));
  using body_ast = typename body::ast;
  using new_api = combined_api<typename binding::api, typename body::api>;
  return extracted_phase<label, new_api, Statement<Let<binding_ast, body_ast>>>{};
}

template <typename l>
template <typename label2, typename _binding, typename Body, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label2, typecheck_phase::Let<_binding, Body>>,
                                             std::enable_if_t<!are_equivalent(label{}, label2{})> const* const)
{
  // binding runs at a different phase, skip it.
  using new_body = DECT(collect_phase(old_api{}, Body{}));
  using body_ast = typename new_body::ast;
  using var = typename _binding::var;
  using new_api = typename new_body::api;

  using stmt = Statement<Sequence<Statement<IncrementOccurance<var>>, body_ast>>;

  return extracted_phase<label, new_api, stmt>{};
}
/*
template <typename l>
template <typename _binding, typename Body, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(
    old_api,
    typecheck_phase::Statement<label,
                               typecheck_phase::LetRemote<_binding, Body>>) {
  using binding = DECT(collect_phase(old_api{}, _binding{}));
  using binding_ast = typename binding::ast;
  using body = DECT(collect_phase(typename binding::api{}, Body{}));
  using body_ast = typename body::ast;
  using new_api =
      combined_api<typename binding::api, typename body::api>;
  return extracted_phase<label, new_api,
                         Statement<LetRemote<binding_ast, body_ast>>>{};
}
//*/
template <typename l>
template <typename label2, typename _binding, typename Body, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label2, typecheck_phase::LetRemote<_binding, Body>>,
                                             std::enable_if_t<!are_equivalent(Label<l>{}, label2{})> const* const)
{
  // binding runs at a different phase, skip it.
	using new_body = DECT(collect_phase(old_api{}, Body{}));
  using body_ast = typename new_body::ast;
  using var = typename _binding::var;
  using new_api = typename new_body::api;

  using stmt = Statement<Sequence<Statement<IncrementRemoteOccurance<var>>, body_ast>>;

  return extracted_phase<label, new_api, stmt>{};
}

template <typename l>
template <typename Var, typename Expr, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label, typecheck_phase::Assignment<Var, Expr>>)
{
  using var = DECT(collect_phase(old_api{}, Var{}));
  using expr = DECT(collect_phase(old_api{}, Expr{}));
  using ast = Statement<Assignment<typename var::ast, typename expr::ast>>;
  return extracted_phase<label, combined_api<typename var::api, typename expr::api>, ast>{};
}
template <typename l>
template <typename Var, typename Expr, typename label2, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label2, typecheck_phase::Assignment<Var, Expr>>,
                                             std::enable_if_t<!are_equivalent(Label<l>{}, label2{})> const* const)
{
  return extracted_phase<label, phase_api<label, requires<>, provides<>, typename old_api::inherits>,
                         Statement<std::conditional_t<label2::flows_to(label{}),
                                                      // we might use this
                                                      IncrementOccurance<typecheck_phase::base_var<Var>>,
                                                      // we can't use this
                                                      Sequence<>>>>{};
}

template <typename l>
template <typename Var,typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label, typecheck_phase::AccompanyWrite<Var>>)
{
  using var = DECT(collect_phase(old_api{}, Var{}));
  using ast = Statement<AccompanyWrite<typename var::ast>>;
  return extracted_phase<label, typename var::api, ast>{};
}
template <typename l>
template <typename Var, typename label2, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label2, typecheck_phase::AccompanyWrite<Var>>,
                                             std::enable_if_t<!are_equivalent(Label<l>{}, label2{})> const* const)
{
  return extracted_phase<label, phase_api<label, requires<>, provides<>, typename old_api::inherits>,Statement<Sequence<>>>{}
}

template <typename l>
template <typename Var,typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label, typecheck_phase::Return<Var>>)
{
  using var = DECT(collect_phase(old_api{}, Var{}));
  using ast = Statement<Return<typename var::ast>>;
  return extracted_phase<label, typename var::api, ast>{};
}
template <typename l>
template <typename Var, typename label2, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label2, typecheck_phase::Return<Var>>,
                                             std::enable_if_t<!are_equivalent(Label<l>{}, label2{})> const* const)
{
  return extracted_phase<label, phase_api<label, requires<>, provides<>, typename old_api::inherits>,Statement<Sequence<>>>{}
}

template <typename l>
template <typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label, typecheck_phase::WriteTombstone>)
{
  return extracted_phase<label,phase_api<label, requires<>, provides<>, typename old_api::inherits>, Statement<WriteTombstone<> > >{};
}
template <typename l>
template <typename label2, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label2, typecheck_phase::WriteTombstone>,
                                             std::enable_if_t<!are_equivalent(Label<l>{}, label2{})> const* const)
{
  return extracted_phase<label, phase_api<label, requires<>, provides<>, typename old_api::inherits>,Statement<Sequence<>>>{}
}

template <typename l>
template <typename cond, typename then, typename old_api, char... name>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<Label<l>, typecheck_phase::While<cond, then, name...>>)
{
  using condition = DECT(collect_phase(old_api{}, cond{}));
  using body = DECT(collect_phase(old_api{}, then{}));
  return extracted_phase<label, DECT(combined_api<typename condition::api, typename body::api>::add_provides(while_binding<name...>{})),
                         Statement<While<typename condition::ast, typename body::ast, name...>>>{};
}

template <typename l>
template <typename label2, typename cond, typename then, typename old_api, char... name>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label2, typecheck_phase::While<cond, then, name...>>,
                                             std::enable_if_t<!are_equivalent(Label<l>{}, label2{}) && label2::flows_to(label{})> const* const)
{
  using body = DECT(collect_phase(old_api{}, then{}));
  // the condition is stronger than you; can enter, but cannot evaluate
  // condition.
  using whilevar = while_binding<name...>;
  static_assert(old_api::provides::template contains<whilevar>() || old_api::inherits::template contains<whilevar>());
  return extracted_phase<label, DECT(body::api::add_requires(whilevar{})), Statement<ForEach<typename body::ast, name...>>>{};
}

template <typename l>
template <typename label2, typename cond, typename then, typename old_api, char... name>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label2, typecheck_phase::While<cond, then, name...>>,
                                             std::enable_if_t<!are_equivalent(Label<l>{}, label2{}) && !label2::flows_to(label{})> const* const)
{
  // the condition is weaker than you; cannot enter
  return extracted_phase<label, phase_api<label, requires<>, provides<>, typename old_api::inherits>, Statement<Sequence<>>>{};
}

template <typename l>
template <typename Yields, typename label2, typename Str, typename Fld, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Expression<label2, Yields, typecheck_phase::FieldReference<Str, Fld>>)
{
  using pre_ast = DECT(collect_phase(old_api{}, Str{}));
  return extracted_phase<label, typename pre_ast::api, Expression<Yields, FieldReference<typename pre_ast::ast, Fld>>>{};
}

template <typename l>
template <int i, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Expression<Label<top>, int, typecheck_phase::Constant<i>>)
{
  return extracted_phase<label, phase_api<label, requires<>, provides<>, typename old_api::inherits>, Expression<int, Constant<i>>>{};
}

template <typename l>
template <char op, typename label2, typename Yields, typename L, typename R, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Expression<label2, Yields, typecheck_phase::BinOp<op, L, R>>)
{
  using left = DECT(collect_phase(old_api{}, L{}));
  using right = DECT(collect_phase(old_api{}, R{}));
  return extracted_phase<label, combined_api<typename left::api, typename right::api>,
                         Expression<Yields, BinOp<op, typename left::ast, typename right::ast>>>{};
}

template <typename l>
template <typename label2, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Expression<label2, tracker::Tombstone, typecheck_phase::GenerateTombstone>)
{
  return extracted_phase<label, phase_api<label,requires<>,provides<>, typename old_api::inherits>,
                         Expression<tracker::Tombstone, GenerateTombstone<>>>>{};
}

template <typename l>
template <typename cond, typename label2, typename _then, typename _els, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label2, typecheck_phase::If<cond, _then, _els>>,
                                             std::enable_if_t<label2::flows_to(label{})> const* const)
{
  using condition = DECT(collect_phase(old_api{}, cond{}));
  using then = DECT(collect_phase(old_api{}, _then{}));
  using els = DECT(collect_phase(old_api{}, _els{}));
  using stmt = If<typename condition::ast, typename then::ast, typename els::ast>;
  return extracted_phase<label, combined_api<combined_api<typename condition::api, typename then::api>, typename els::api>, Statement<stmt>>{};
}

template <typename l>
template <typename cond, typename label2, typename _then, typename _els, typename old_api>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label2, typecheck_phase::If<cond, _then, _els>>,
                                             std::enable_if_t<!label2::flows_to(label{})> const* const)
{
  // can't enter, guarding expression is insufficintly trusted.
  return extracted_phase<label, phase_api<label, requires<>, provides<>, typename old_api::inherits>, Statement<Sequence<>>>{};
}

template <typename l, typename inherits>
constexpr auto build_seq2()
{
  return phase_api<l, requires<>, provides<>, inherits>{};
}

template <typename label, typename inherits, typename seq1, typename... processed_seq>
constexpr auto build_seq2(const seq1&, const processed_seq&... o)
{
  return combined_api<typename seq1::api, DECT(build_seq2<label, inherits>(o...))>{};
}

template <typename T>
using ast_of = typename T::ast;
template <typename label, typename inherits, typename... seq>
constexpr auto build_seq1(const seq&... ps)
{
  using Seq = typename AST<label>::template Sequence<ast_of<seq>...>;
  using Stmt = DECT(AST<label>::collapse(typename AST<label>::template Statement<Seq>{}));
  return extracted_phase<label, DECT(build_seq2<label, inherits>(ps...)), Stmt>{};
}

template <typename l>
template <typename label2, typename old_api, typename... seq>
constexpr auto AST<Label<l>>::_collect_phase(old_api, typecheck_phase::Statement<label2, typecheck_phase::Sequence<seq...>>)
{
  return build_seq1<label, typename old_api::inherits>(collect_phase(old_api{}, seq{})...);
}

template <typename label, typename ast, typename oldinherit>
constexpr auto collect_phase(ast, oldinherit)
{
  return AST<label>::collect_phase(phase_api<label, requires<>, provides<>, oldinherit>{}, ast{});
}

template <typename AST>
constexpr auto split_computation(AST a);

constexpr bool
labels_in_descending_order()
{
  return true;
}

template <typename lbl1, typename... Labels>
constexpr bool
labels_in_descending_order(Label<lbl1>, const Labels&... l)
{
  return Label<lbl1>::is_max_of(l...) && labels_in_descending_order(l...);
}

template <typename AST, typename oldinherit>
constexpr auto _split_computation(AST , mutils::typelist<> )
{
  return runnable_transaction::pretransaction<>{};
}

template <typename AST, typename oldinherit, typename curr_label, typename... Labels>
constexpr auto _split_computation(AST a, mutils::typelist<curr_label, Labels...>)
{
  static_assert(labels_in_descending_order(Labels{}...));
  using curr_phase = DECT(remove_unused(collect_phase<curr_label>(a, oldinherit{})));
  static_assert(std::is_same<typename curr_phase::label, curr_label>::value);
  using provides = typename curr_phase::api::provides;
  using next_inherits = DECT(curr_phase::api::inherits::addAll(typename provides::as_typelist{}));
  using empty_cleared = DECT(clear_empty_statements<curr_label>(typename curr_phase::ast{}));
  using newAST = typename empty_cleared::ast;
  using no_longer_require = DECT(empty_cleared::remove_from_require::subtract(typename empty_cleared::still_require{}));
  using edited_phase =
    extracted_phase<curr_label,
                    phase_api<curr_label, DECT(to_requires(without_names(no_longer_require{}, to_typeset(typename curr_phase::api::requires::as_typelist{})))),
                              typename curr_phase::api::provides, typename curr_phase::api::inherits>,
                    newAST>;
  using final_phase = DECT(remove_unused(edited_phase{}));
  return runnable_transaction::pretransaction<DECT(transaction_prephase(final_phase{}))>::append(
    _split_computation<AST, next_inherits, Labels...>(a, mutils::typelist<Labels...>{}));
}
	
	template <txnID_t id, typename AST, typename... bindings>
constexpr auto split_computation()
{
  constexpr auto a = typecheck_phase::name_while<1, 1>(AST{});
  using labels = DECT(collect_proper_labels(AST{}).reverse());
  return typename DECT(_split_computation<DECT(a), inherits<bindings...>>(a, labels{}))
		::template processed<id>{};
}
}
}
}
