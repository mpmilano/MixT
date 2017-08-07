#pragma once

#include "top.hpp"
#include "AST_typecheck.hpp"

namespace myria {
namespace mtl {
namespace typecheck_phase {
namespace label_inference {

template <typename, typename>
struct replace_label;

template <int target1, int target2, typename newlabel>
struct replace_label<Label<temp_label<target1, target2>>, Label<newlabel>>
{
  using old_label = Label<temp_label<target1, target2>>;
  using new_label = Label<newlabel>;

  template <typename ast>
  constexpr static auto replace(ast);
  template <typename l>
  constexpr static auto label_replace(l);

  constexpr static auto _label_replace(old_label) { return new_label{}; }

  template <typename l>
  constexpr static auto _label_replace(l, std::enable_if_t<!std::is_same<l, old_label>::value>* = nullptr)
  {
    return l{};
  }

  template <typename l, typename r>
  constexpr static auto _label_replace(Label<label_min_of<l, r>>)
  {
    return Label<label_min_of<DECT(label_replace(l{})), DECT(label_replace(r{}))>>::resolve();
  }

  template <typename l, typename y, typename v, typename e>
  static constexpr auto _replace(Binding<l, y, v, e>)
  {
    return Binding<DECT(label_replace(l{})), y, v, DECT(replace(e{}))>{};
  }

  template <typename l, typename y, typename s, typename f>
  static constexpr auto _replace(Expression<l, y, FieldReference<s, f>>)
  {
    return Expression<DECT(label_replace(l{})), y, FieldReference<DECT(replace(s{})), f>>{};
  }

  template <typename l, typename y, typename v>
  static constexpr auto _replace(Expression<l, y, VarReference<v>>)
  {
    return Expression<DECT(label_replace(l{})), y, VarReference<v>>{};
  }

  template <int i>
  static constexpr auto _replace(Expression<Label<top>, int, Constant<i>> a)
  {
    static_assert(!std::is_same<old_label, Label<top>>::value);
    return a;
  }

  template <typename l, typename y, char op, typename L, typename R>
  static constexpr auto _replace(Expression<l, y, BinOp<op, L, R>>)
  {
    return Expression<DECT(label_replace(l{})), y, BinOp<op, DECT(replace(L{})), DECT(replace(R{}))>>{};
  }

  template <typename l, typename b, typename e>
  static constexpr auto _replace(Statement<l, Assignment<b, e>>)
  {
    return Statement<DECT(label_replace(l{})), Assignment<DECT(replace(b{})), DECT(replace(e{}))>>{};
  }

	template <typename l, typename e>
  static constexpr auto _replace(Statement<l, Return<e>>)
  {
    return Statement<DECT(label_replace(l{})), Return<DECT(replace(e{}))>>{};
  }

  template <typename l, typename b, typename e>
  static constexpr auto _replace(Statement<l, Let<b, e>>)
  {
    return Statement<DECT(label_replace(l{})), Let<DECT(replace(b{})), DECT(replace(e{}))>>{};
  }

  template <typename l, typename b, typename e>
  static constexpr auto _replace(Statement<l, LetRemote<b, e>>)
  {
    return Statement<DECT(label_replace(l{})), LetRemote<DECT(replace(b{})), DECT(replace(e{}))>>{};
  }

	template <typename l, typename n, typename h, typename e>
		static constexpr auto _replace(Statement<l, LetIsValid<n,h, e>>)
  {
    return Statement<DECT(label_replace(l{})), LetIsValid<n,DECT(replace(h{})), DECT(replace(e{}))>>{};
  }

	template <typename l, typename oper_name, typename Hndl, typename... args>
		static constexpr auto _replace(Statement<l, StatementOperation<oper_name, Hndl, args...>>)
  {
	  return Statement<DECT(label_replace(l{})), StatementOperation<oper_name,DECT(replace(Hndl{})), DECT(replace(args{}))...>>{};
  }

  template <typename l, typename c, typename t, typename e>
  static constexpr auto _replace(Statement<l, If<c, t, e>>)
  {
    return Statement<DECT(label_replace(l{})), If<DECT(replace(c{})), DECT(replace(t{})), DECT(replace(e{}))>>{};
  }

  template <typename l, typename c, typename e>
  static constexpr auto _replace(Statement<l, While<c, e>>)
  {
    return Statement<DECT(label_replace(l{})), While<DECT(replace(c{})), DECT(replace(e{}))>>{};
  }

  template <typename l, typename... seq>
  static constexpr auto _replace(Statement<l, Sequence<seq...>>)
  {
    return Statement<DECT(label_replace(l{})), Sequence<DECT(replace(seq{}))...>>{};
  }
};

template <int target1, int target2, typename newlabel>
template <typename ast>
constexpr auto replace_label<Label<temp_label<target1, target2>>, Label<newlabel>>::replace(ast a)
{
  return replace_label::_replace(a);
}

template <int target1, int target2, typename newlabel>
template <typename l>
constexpr auto replace_label<Label<temp_label<target1, target2>>, Label<newlabel>>::label_replace(l)
{
  return _label_replace(l{});
}
}
}
}
}
