#pragma once

#include "top.hpp"
#include "AST_typecheck.hpp"
#include "replace_label.hpp"
#include "collect_proper_label.hpp"

namespace myria {
namespace mtl {
namespace typecheck_phase {
namespace label_inference {

template <int l1, int l2, int r1, int r2>
constexpr bool less_than(Label<temp_label<l1, l2>>, Label<temp_label<r1, r2>>)
{
  return (l1 < r1 ? true : l2 < r2);
}

template <int l1, int l2, typename l>
constexpr bool less_than(Label<l>, Label<temp_label<l1, l2>>)
{
  return true;
}

template <int l1, int l2, typename l>
constexpr bool less_than(Label<temp_label<l1, l2>>, Label<l>)
{
  return false;
}

template <typename l1, typename l2>
constexpr bool
less_than(Label<l1> a, Label<l2> b)
{
  return b.flows_to(a);
}

  template <typename high, typename low, typename why>
struct must_flow_to
{
  constexpr must_flow_to() = default;

  template <typename newl, typename oldl>
  static constexpr auto substitute()
  {
    return must_flow_to<std::conditional_t<std::is_same<oldl, high>::value, newl, high>, std::conditional_t<std::is_same<oldl, low>::value, newl, low>, why>{};
  }

  template <typename high2, typename low2, typename why2>
  static constexpr bool lte(must_flow_to<high2, low2,why2>)
  {
    if (is_temp_label<high>::value) {
      if (is_temp_label<high2>::value) {
        return less_than(low{}, low2{});
      } else
        return false;
    } else if (is_temp_label<high2>::value) {
      return true;
    } else
      return (less_than(high{}, high2{}) ? true : less_than(low{}, low2{}));
  }
};

template <typename... t>
constexpr auto constraints_from_typeset(mutils::typeset<t...>);

template <typename... t>
constexpr auto constraints_from_typelist(mutils::typelist<t...>);

template <typename... A>
struct constraints
{
  constexpr constraints() = default;
  template <typename... T>
  static constexpr auto append(constraints<T...>)
  {
    return constraints<A..., T...>{};
  }

  template <typename L, typename R>
  using lte = std::integral_constant<bool, L::lte(R{})>;
  static constexpr auto sort()
  {
    using namespace mutils;
    return constraints_from_typelist(typelist<A...>::template sort<lte>());
  }

  template <typename newl, typename oldl>
  static constexpr auto substitute()
  {
    return constraints<DECT(A::template substitute<newl, oldl>())...>{};
  }
};

template <typename... t>
constexpr auto constraints_from_typeset(mutils::typeset<t...>)
{
  return constraints<t...>{};
}

template <typename... t>
constexpr auto constraints_from_typelist(mutils::typelist<t...>)
{
  return constraints<t...>{};
}

  template <typename high, typename low, typename why>
std::ostream& operator<<(std::ostream& o, must_flow_to<high, low,why>)
{
  return o << high{} << " ==> " << low{} << "  (" << why{} << ")";
}

template <typename... A>
std::ostream& operator<<(std::ostream& o, constraints<A...>)
{
  o << "constraints: " << std::endl;
  static const auto print = [&](const auto& e) {
    o << e << std::endl;
    return nullptr;
  };
  (print(A{}), ... );
  return o;
}

template <typename pc_label, typename ast>
constexpr auto collect_constraints(Label<pc_label>, ast);

template <typename pc_label, typename l, typename y, typename ye, typename v, typename le, typename e>
constexpr auto _collect_constraints(Binding<l, y, v, Expression<le, ye, e>>)
{
  return constraints<must_flow_to<le, l, MUTILS_STRING(bound expression)>, must_flow_to<pc_label, l, MUTILS_STRING(bound expression, pc)>>::append(collect_constraints(pc_label{}, Expression<le, y, e>{}));
}

template <typename pc_label, typename l, typename y, typename s, typename f>
constexpr auto _collect_constraints(Expression<l, y, FieldReference<s, f>>)
{
  return collect_constraints(pc_label{}, s{});
}

template <typename pc_label, typename l, typename y, typename v>
constexpr auto _collect_constraints(Expression<l, y, VarReference<v>>)
{
  return constraints<>{};
}

template <typename pc_label, int i>
constexpr auto _collect_constraints(Expression<Label<top>, int, Constant<i>>)
{
  return constraints<>{};
}

template <typename pc_label, typename l, typename y, char op, typename L, typename R>
constexpr auto _collect_constraints(Expression<l, y, BinOp<op, L, R>>)
{
  return collect_constraints(pc_label{}, L{}).append(collect_constraints(pc_label{}, R{}));
}

template <typename pc_label, typename l, typename lv, typename yv, typename v, typename le, typename ye, typename e>
constexpr auto _collect_constraints(Statement<l, Assignment<Expression<lv, yv, v>, Expression<le, ye, e>>>)
{
  return constraints<must_flow_to<le, lv,MUTILS_STRING(assignment)>, must_flow_to<pc_label, lv,MUTILS_STRING(assignment, pc)>>::append(collect_constraints(pc_label{}, Expression<lv, yv, v>{}))
    .append(collect_constraints(pc_label{}, Expression<le, ye, e>{}));
}

template <typename pc_label, typename l, typename le, typename ye, typename e>
constexpr auto _collect_constraints(Statement<l, Return<Expression<le, ye, e>>>)
{
  return collect_constraints(pc_label{}, Expression<le, ye, e>{});
}

template <typename pc_label, typename l, typename b, typename e>
constexpr auto _collect_constraints(Statement<l, Let<b, e>>)
{
  return collect_constraints(pc_label{}, b{}).append(collect_constraints(pc_label{}, e{}));
}

template <typename pc_label, typename l, typename b, typename e>
constexpr auto _collect_constraints(Statement<l, LetRemote<b, e>> a)
{
  using This = DECT(a);
  using new_pc = Label<label_min_of<Label<label_min_of<pc_label, typename This::expr_label>>, typename This::handle_label>>;
  return collect_constraints(new_pc{}, b{})
    .append(collect_constraints(new_pc{}, e{}))
    .append(constraints<must_flow_to<typename This::expr_label, typename This::handle_label,MUTILS_STRING(let_remote, expr -> hndl)>>{});
}

template <typename pc_label, typename l, typename c, typename t, typename e>
constexpr auto _collect_constraints(Statement<l, If<c, t, e>>)
{
  using new_pc = Label<label_min_of<pc_label, l>>;
  return collect_constraints(new_pc{}, c{}).append(collect_constraints(new_pc{}, t{})).append(collect_constraints(new_pc{}, e{}));
}

template <typename pc_label, typename l, typename c, typename e>
constexpr auto _collect_constraints(Statement<l, While<c, e>>)
{
  using new_pc = Label<label_min_of<pc_label, l>>;
  return collect_constraints(new_pc{}, c{}).append(collect_constraints(new_pc{}, e{}));
}

template <typename pc_label, typename l>
constexpr auto _collect_constraints(Statement<l, Sequence<>>)
{
  return constraints<>{};
}

template <typename pc_label, typename l, typename s1, typename... seq>
constexpr auto _collect_constraints(Statement<l, Sequence<s1, seq...>>)
{
  return collect_constraints(pc_label{}, s1{}).append(collect_constraints(pc_label{}, Statement<l, Sequence<seq...>>{}));
}

template <typename pc_label, typename ast>
constexpr auto collect_constraints(Label<pc_label>, ast a)
{
  return _collect_constraints<Label<pc_label>>(a);
}

  template <typename to, typename l, typename r, typename why, typename... rest>
  constexpr auto collapse_constraints(constraints<must_flow_to<Label<label_min_of<l, r> >, to, why>, rest...>)
{
  return collapse_constraints(constraints<must_flow_to<l, to,why>, must_flow_to<r, to,why>, rest...>{});
}

template <typename>
struct is_min_of;
template <typename l, typename r>
struct is_min_of<Label<label_min_of<l, r>>> : public std::true_type
{
};
template <typename l, typename r>
struct is_min_of<label_min_of<l, r>> : public std::true_type
{
};
template <typename>
struct is_min_of : public std::false_type
{
};

  template <typename from, typename to, typename why, typename... rst>
  constexpr auto collapse_constraints(constraints<must_flow_to<from, to, why>, rst...>, std::enable_if_t<!(is_min_of<from>::value || is_min_of<to>::value)>* = nullptr)
{
  return constraints<must_flow_to<from, to, why>>::append(collapse_constraints(constraints<rst...>{}));
}

constexpr auto collapse_constraints(constraints<> a)
{
  return a;
}

constexpr auto remove_reflexive_constraints(constraints<> a)
{
  return a;
}

  template <typename l, typename l2, typename why, typename... t>
  constexpr auto remove_reflexive_constraints(constraints<must_flow_to<l, l2,why>, t...>, std::enable_if_t<!std::is_same<l, l2>::value>* = nullptr)
{
  return constraints<must_flow_to<l, l2, why>>::append(remove_reflexive_constraints(constraints<t...>{}));
}

  template <typename l, typename why, typename... t>
  constexpr auto remove_reflexive_constraints(constraints<must_flow_to<l, l,why>, t...>)
{
  return remove_reflexive_constraints(constraints<t...>{});
}

template <typename... t>
constexpr auto minimize_constraints(constraints<t...>)
{
  using namespace mutils;
  return remove_reflexive_constraints(constraints_from_typeset(typeset<>::add<t...>()));
}

template <typename _ast, typename label>
constexpr auto replace_all_less_than(constraints<>)
{
  using _constraints = constraints<>;
  {
    using namespace mutils;
    struct ret
    {
      constexpr ret() = default;
      using ast = _ast;
      using constraints = _constraints;
      using substitutions = typelist<>;
    };
    return ret{};
  }
}

template <typename newl, typename oldl>
struct substitution
{
};

template <typename newl, typename oldl>
std::ostream& operator<<(std::ostream& o, substitution<newl, oldl>)
{
  return o << oldl{} << " ==> " << newl{};
}

  template <typename _ast, typename label, int to1, int to2, typename why, typename... constraint>
  constexpr auto replace_all_less_than(constraints<must_flow_to<label, Label<temp_label<to1, to2> >,why >, constraint...>)
{
  static_assert(!std::is_same<label, Label<temp_label<to1, to2>>>::value);
  //"to" needs to be smaller than smallest label.
  using new_ast = DECT(replace_label<Label<temp_label<to1, to2>>, label>::replace(_ast{}));
  using intermediate =
    DECT(replace_all_less_than<new_ast, label>(constraints<constraint...>::template substitute<label, Label<temp_label<to1, to2>>>().sort()));
  struct ret
  {
    constexpr ret() = default;
    using ast = typename intermediate::ast;
    using substitutions = DECT(intermediate::substitutions::append(mutils::typelist<substitution<label, Label<temp_label<to1, to2>>>>{}));
    using constraints = DECT(minimize_constraints(constraints<must_flow_to<label, label, MUTILS_STRING(this seems wrong)>>::append(typename intermediate::constraints{})));
  };
  return ret{};
}

  template <typename _ast, typename label, typename to, typename why, typename... constraint>
  constexpr auto replace_all_less_than(constraints<must_flow_to<label, to, why>, constraint...>)
{
  // this is an actual concrete pair of labels,
  // so let's check them.
  using intermediate = DECT(replace_all_less_than<_ast, label>(constraints<constraint...>{}));
  static_assert(mutils::useful_static_assert<label::flows_to(to{}),typename intermediate::ast>(), "Error: flow violation");
  struct ret
  {
    constexpr ret() = default;
    using ast = typename intermediate::ast;
    using substitutions = typename intermediate::substitutions;
    using constraints = DECT(minimize_constraints(constraints<must_flow_to<label, to, why>>::append(typename intermediate::constraints{})));
  };
  return ret{};
}

  template <typename _ast, typename label, typename from, typename to, typename why, typename... constraint>
constexpr auto replace_all_less_than(constraints<must_flow_to<from, to, why>, constraint...>, std::enable_if_t<!std::is_same<label, from>::value>* = nullptr)
{
  static_assert(!std::is_same<label, from>::value);
  using intermediate = DECT(replace_all_less_than<_ast, label>(constraints<constraint...>{}));
  struct ret
  {
    constexpr ret() = default;
    using ast = typename intermediate::ast;
    using substitutions = typename intermediate::substitutions;
    // this constraint operates at a higher label than we do, so keep it around until
    // that higher label gets a turn.
    using constraints = DECT(minimize_constraints(constraints<must_flow_to<from, to, why >>::append(typename intermediate::constraints{})));
  };
  return ret{};
}

template <typename _ast, typename _constraints>
constexpr auto infer_labels_helper1(mutils::typelist<>)
{
  struct ret
  {
    constexpr ret() = default;
    using ast = _ast;
    using constraints = _constraints;
  };
  return ret{};
}

template <typename ast, typename constraints, typename label1, typename... labels>
constexpr auto infer_labels_helper1(mutils::typelist<label1, labels...>)
{
  using namespace mutils;
  using pair = DECT(replace_all_less_than<ast, label1>(constraints::sort()));
  using new_ast = typename pair::ast;
  using new_constraints = typename pair::constraints;
  return infer_labels_helper1<new_ast, new_constraints>(typelist<labels...>{});
}

template <typename ast>
constexpr auto infer_labels(ast)
{
  constexpr auto real_labels = collect_proper_labels(ast{});
  using constraints = DECT(minimize_constraints(collapse_constraints(collect_constraints(Label<top>{}, ast{}))));
  return typename DECT(infer_labels_helper1<ast, constraints>(real_labels))::ast{};
}
}
}
}
}
