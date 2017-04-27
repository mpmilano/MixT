#pragma once
#include "AST_split.hpp"

namespace myria {
namespace mtl {
namespace split_phase {

template <typename l, typename AST>
auto clear_empty_statements(AST a);

template <typename l, typename y, typename s, typename f>
auto _clear_empty_statements(typename AST<l>::template Expression<y, typename AST<l>::template FieldReference<s, f>>)
{
  using strct = DECT(clear_empty_statements<l>(s{}));
  struct ret
  {
    using ast = typename AST<l>::template Expression<y, typename AST<l>::template FieldReference<typename strct::ast, f>>;
    using remove_from_require = typename strct::remove_from_require;
    using still_require = typename strct::still_require;
  };
  return ret{};
}

template <typename l, typename y, typename v>
auto _clear_empty_statements(typename AST<l>::template Expression<y, typename AST<l>::template VarReference<v>> a)
{
  struct ret
  {
    using ast = DECT(a);
    using remove_from_require = mutils::typeset<>;
    using still_require = mutils::typeset<v>;
  };
  return ret{};
}

template <typename l, int i>
auto _clear_empty_statements(typename AST<l>::template Expression<int, typename AST<l>::template Constant<i>> a)
{
  struct ret
  {
    using ast = DECT(a);
    using remove_from_require = mutils::typeset<>;
    using still_require = mutils::typeset<>;
  };
  return ret{};
}

template <typename l>
auto _clear_empty_statements(typename AST<l>::template Expression<tracker::Tombstone, typename AST<l>::template GenerateTombstone<>> a)
{
  struct ret
  {
    using ast = DECT(a);
    using remove_from_require = mutils::typeset<>;
    using still_require = mutils::typeset<>;
  };
  return ret{};
}

template <typename l, typename y, char op, typename L, typename R>
auto _clear_empty_statements(typename AST<l>::template Expression<y, typename AST<l>::template BinOp<op, L, R>>)
{
  using newl = DECT(clear_empty_statements<l>(L{}));
  using newr = DECT(clear_empty_statements<l>(R{}));
  struct ret
  {
    using ast = typename AST<l>::template Expression<y, typename AST<l>::template BinOp<op, typename newl::ast, typename newr::ast>>;
    using remove_from_require = DECT(newl::remove_from_require::combine(typename newr::remove_from_require{}));
    using still_require = DECT(newl::still_require::combine(typename newr::still_require{}));
  };
  return ret{};
}

template <typename l, typename l2, typename y, typename v, typename e>
auto _clear_empty_statements(typename AST<l>::template Binding<l2, y, v, e>)
{
  using newe = DECT(clear_empty_statements<l>(e{}));
  struct ret
  {
    using ast = typename AST<l>::template Binding<l2, y, v, typename newe::ast>;
    using remove_from_require = typename newe::remove_from_require;
    using still_require = typename newe::still_require;
  };
  return ret{};
}

template <typename l, typename b, typename body>
auto _clear_empty_statements(typename AST<l>::template Statement<typename AST<l>::template Let<b, body>>)
{
  using newb = DECT(clear_empty_statements<l>(b{}));
  using new_body = DECT(clear_empty_statements<l>(body{}));
  struct ret
  {
    using ast = typename AST<l>::template Statement<typename AST<l>::template Let<typename newb::ast, typename new_body::ast>>;
    using remove_from_require = DECT(newb::remove_from_require::combine(typename new_body::remove_from_require{}));
    using still_require = DECT(newb::still_require::combine(typename new_body::still_require{}));
  };
  return ret{};
}

template <typename l, typename b, typename body>
auto _clear_empty_statements(typename AST<l>::template Statement<typename AST<l>::template LetRemote<b, body>>)
{
  using newb = DECT(clear_empty_statements<l>(b{}));
  using new_body = DECT(clear_empty_statements<l>(body{}));
  struct ret
  {
    using ast = typename AST<l>::template Statement<typename AST<l>::template LetRemote<typename newb::ast, typename new_body::ast>>;
    using remove_from_require = DECT(newb::remove_from_require::combine(typename new_body::remove_from_require{}));
    using still_require = DECT(newb::still_require::combine(typename new_body::still_require{}));
  };
  return ret{};
}

	template <typename l, typename n, typename h, typename body>
	auto _clear_empty_statements(typename AST<l>::template Statement<typename AST<l>::template LetIsValid<n,h, body>>)
{
  using newh = DECT(clear_empty_statements<l>(h{}));
  using new_body = DECT(clear_empty_statements<l>(body{}));
  struct ret
  {
    using ast = typename AST<l>::template Statement<typename AST<l>::template LetIsValid<n, typename newh::ast, typename new_body::ast>>;
    using remove_from_require = DECT(newh::remove_from_require::combine(typename new_body::remove_from_require{}));
    using still_require = DECT(newh::still_require::combine(typename new_body::still_require{}));
  };
  return ret{};
}
	
template <typename l, typename L, typename R>
auto _clear_empty_statements(typename AST<l>::template Statement<typename AST<l>::template Assignment<L, R>>)
{
  using newl = DECT(clear_empty_statements<l>(L{}));
  using newr = DECT(clear_empty_statements<l>(R{}));
  struct ret
  {
    using ast = typename AST<l>::template Statement<typename AST<l>::template Assignment<typename newl::ast, typename newr::ast>>;
    using remove_from_require = DECT(newl::remove_from_require::combine(typename newr::remove_from_require{}));
    using still_require = DECT(newl::still_require::combine(typename newr::still_require{}));
  };
  return ret{};
}
	
template <typename l, typename R>
auto _clear_empty_statements(typename AST<l>::template Statement<typename AST<l>::template Return<R>>)
{
  using newr = DECT(clear_empty_statements<l>(R{}));
  struct ret
  {
    using ast = typename AST<l>::template Statement<typename AST<l>::template Return<typename newr::ast>>;
    using remove_from_require = typename newr::remove_from_require;
    using still_require = typename newr::still_require;
  };
  return ret{};
}

template <typename l, typename R>
auto _clear_empty_statements(typename AST<l>::template Statement<typename AST<l>::template AccompanyWrite<R>>)
{
  using newr = DECT(clear_empty_statements<l>(R{}));
  struct ret
  {
    using ast = typename AST<l>::template Statement<typename AST<l>::template AccompanyWrite<typename newr::ast>>;
    using remove_from_require = typename newr::remove_from_require;
    using still_require = typename newr::still_require;
  };
  return ret{};
}

template <typename l, char... var>
auto _clear_empty_statements(typename AST<l>::template Statement<typename AST<l>::template IncrementOccurance<String<var...>>> a)
{
  struct ret
  {
    using ast = DECT(a);
    using remove_from_require = mutils::typeset<>;
    using still_require = mutils::typeset<>;
  };
  return ret{};
}

  template <typename l, typename T>
auto _clear_empty_statements(typename AST<l>::template Statement<typename AST<l>::template WriteTombstone<T> >)
{
  using newr = DECT(clear_empty_statements<l>(T{}));
  struct ret
  {
    using ast = typename AST<l>::template Statement<typename AST<l>::template WriteTombstone<typename newr::ast>>;
    using remove_from_require = typename newr::remove_from_require;
    using still_require = typename newr::still_require;
  };
  return ret{};
}

template <typename l, char... var>
auto _clear_empty_statements(typename AST<l>::template Statement<typename AST<l>::template IncrementRemoteOccurance<String<var...>>> a)
{
  struct ret
  {
    using ast = DECT(a);
    using remove_from_require = mutils::typeset<>;
    using still_require = mutils::typeset<>;
  };
  return ret{};
}

template <typename l, typename c, typename t, typename e>
auto _clear_empty_statements(typename AST<l>::template Statement<typename AST<l>::template If<c, t, e>>)
{
  using newc = DECT(clear_empty_statements<l>(c{}));
  using newt = DECT(clear_empty_statements<l>(t{}));
  using newe = DECT(clear_empty_statements<l>(e{}));
  constexpr bool empty_if = AST<l>::template is_empty_sequence<typename newt::ast>::value && AST<l>::template is_empty_sequence<typename newe::ast>::value;
  using stmt = std::conditional_t<empty_if, typename AST<l>::template Sequence<>,
                                  typename AST<l>::template If<typename newc::ast, typename newt::ast, typename newe::ast>>;
  struct ret
  {
    using ast = typename AST<l>::template Statement<stmt>;
    using remove_from_require =
      std::conditional_t<empty_if, typename newc::still_require,
                         DECT(newc::remove_from_require::combine(typename newt::remove_from_require{}).combine(typename newe::remove_from_require{}))>;
    using still_require = std::conditional_t<empty_if, mutils::typeset<>,
                                             DECT(newc::still_require::combine(typename newt::still_require{}).combine(typename newe::still_require{}))>;
  };
  return ret{};
}

template <typename l, typename c, typename t, char... name>
auto _clear_empty_statements(typename AST<l>::template Statement<typename AST<l>::template While<c, t, name...>>)
{
  using newc = DECT(clear_empty_statements<l>(c{}));
  using newt = DECT(clear_empty_statements<l>(t{}));

  struct ret
  {
    using ast = typename AST<l>::template Statement<typename AST<l>::template While<typename newc::ast, typename newt::ast, name...>>;
    using remove_from_require = DECT(newc::remove_from_require::combine(typename newt::remove_from_require{}));
    using still_require = DECT(newc::still_require::combine(typename newt::still_require{}));
  };
  return ret{};
}

template <typename l, typename t, char... name>
auto _clear_empty_statements(typename AST<l>::template Statement<typename AST<l>::template ForEach<t, name...>>)
{
  using newt = DECT(clear_empty_statements<l>(t{}));
  struct ret
  {
    using ast = typename AST<l>::template Statement<typename AST<l>::template ForEach<typename newt::ast, name...>>;
    using remove_from_require = typename newt::remove_from_require;
    using still_require = typename newt::still_require;
  };
  return ret{};
}

template <typename l, typename... Seq>
auto _clear_empty_statements(typename AST<l>::template Statement<typename AST<l>::template Sequence<Seq...>>)
{
  using namespace mutils;
  struct ret
  {
    using ast = DECT(
      AST<l>::collapse(typename AST<l>::template Statement<typename AST<l>::template Sequence<typename DECT(clear_empty_statements<l>(Seq{}))::ast...>>{}));
    using remove_from_require = DECT(typelist_ns::combine(typename DECT(clear_empty_statements<l>(Seq{}))::remove_from_require{}...));
    using still_require = DECT(typelist_ns::combine(typename DECT(clear_empty_statements<l>(Seq{}))::still_require{}...));
  };
  return ret{};
}

template <typename l, typename AST>
auto clear_empty_statements(AST a)
{
  return _clear_empty_statements<l>(a);
}
}
}
}
