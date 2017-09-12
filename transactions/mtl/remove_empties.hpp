#pragma once
#include "mtl/AST_split.hpp"

namespace myria {
namespace mtl {
namespace split_phase {

	BEGIN_SPLIT_CONTEXT(remove_empties);
	
template <typename AST>
static auto clear_empty_statements(AST a);

template <typename y, typename s, typename f>
static auto _clear_empty_statements(Expression<y, FieldReference<s, f>>)
{
  using strct = DECT(clear_empty_statements(s{}));
  struct ret
  {
    using ast = Expression<y, FieldReference<typename strct::ast, f>>;
    using remove_from_require = typename strct::remove_from_require;
    using still_require = typename strct::still_require;
  };
  return ret{};
}

template <typename y, typename v>
static auto _clear_empty_statements(Expression<y, VarReference<v>> a)
{
  struct ret
  {
    using ast = DECT(a);
    using remove_from_require = mutils::typeset<>;
    using still_require = mutils::typeset<v>;
  };
  return ret{};
}

template <int i>
static auto _clear_empty_statements(Expression<int, Constant<i>> a)
{
  struct ret
  {
    using ast = DECT(a);
    using remove_from_require = mutils::typeset<>;
    using still_require = mutils::typeset<>;
  };
  return ret{};
}

static auto _clear_empty_statements(Expression<tracker::Tombstone, GenerateTombstone> a)
{
  struct ret
  {
    using ast = DECT(a);
    using remove_from_require = mutils::typeset<>;
    using still_require = mutils::typeset<>;
  };
  return ret{};
}

	template <typename y, char op, typename L, typename R>
static auto _clear_empty_statements(Expression<y, BinOp<op, L, R>>)
{
  using newl = DECT(clear_empty_statements(L{}));
  using newr = DECT(clear_empty_statements(R{}));
  struct ret
  {
    using ast = Expression<y, BinOp<op, typename newl::ast, typename newr::ast>>;
    using remove_from_require = DECT(newl::remove_from_require::combine(typename newr::remove_from_require{}));
    using still_require = DECT(newl::still_require::combine(typename newr::still_require{}));
  };
  return ret{};
}

	
	template <typename y, typename h>
	static auto _clear_empty_statements(Expression<y,IsValid<h>>)
{
  using newh = DECT(clear_empty_statements(h{}));
  struct ret
  {
	  using ast = Expression<y,IsValid<typename newh::ast>>;
    using remove_from_require = typename newh::remove_from_require;
    using still_require = typename newh::still_require;
  };
  return ret{};
}

	template <typename n, typename h, typename... a>
	static auto _clear_empty_statements(Operation<n,h, a...>)
{
  using newh = DECT(clear_empty_statements(h{}));
  struct ret
  {
	  using ast = Operation<n, typename newh::ast, typename DECT(clear_empty_statements(a{}))::ast...>;
	  using remove_from_require = DECT(newh::remove_from_require::combine(typename DECT(clear_empty_statements(a{}))::remove_from_require{}...));
	  using still_require = DECT(newh::still_require::combine(typename DECT(clear_empty_statements(a{}))::still_require{}...));
  };
  return ret{};
}
	
	template <typename n, typename h, typename... a>
	static auto _clear_empty_statements(Statement<Operation<n,h, a...>>)
{
	using ret_p = DECT(_clear_empty_statements(Operation<n,h, a...>{}));
  struct ret
  {
	  using ast = Statement<typename ret_p::ast >;
	  using remove_from_require = typename ret_p::remove_from_require;
	  using still_require = typename ret_p::still_require;
  };
  return ret{};
}
	template <typename y, typename n, typename h, typename... a>
	static auto _clear_empty_statements(Expression<y,Operation<n,h, a...>>)
{
	using ret_p = DECT(_clear_empty_statements(Operation<n,h, a...>{}));
  struct ret
  {
	  using ast = Expression<y,typename ret_p::ast >;
	  using remove_from_require = typename ret_p::remove_from_require;
	  using still_require = typename ret_p::still_require;
  };
  return ret{};
}

template <typename l2, typename y, typename v, typename e>
static auto _clear_empty_statements(Binding<l2, y, v, e>)
{
  using newe = DECT(clear_empty_statements(e{}));
  struct ret
  {
    using ast = Binding<l2, y, v, typename newe::ast>;
    using remove_from_require = typename newe::remove_from_require;
    using still_require = typename newe::still_require;
  };
  return ret{};
}

template <typename b, typename body>
static auto _clear_empty_statements(Statement<Let<b, body>>)
{
  using newb = DECT(clear_empty_statements(b{}));
  using new_body = DECT(clear_empty_statements(body{}));
  struct ret
  {
    using ast = Statement<Let<typename newb::ast, typename new_body::ast>>;
    using remove_from_require = DECT(newb::remove_from_require::combine(typename new_body::remove_from_require{}));
    using still_require = DECT(newb::still_require::combine(typename new_body::still_require{}));
  };
  return ret{};
}

template <typename b, typename body>
static auto _clear_empty_statements(Statement<LetRemote<b, body>>)
{
  using newb = DECT(clear_empty_statements(b{}));
  using new_body = DECT(clear_empty_statements(body{}));
  struct ret
  {
    using ast = Statement<LetRemote<typename newb::ast, typename new_body::ast>>;
    using remove_from_require = DECT(newb::remove_from_require::combine(typename new_body::remove_from_require{}));
    using still_require = DECT(newb::still_require::combine(typename new_body::still_require{}));
  };
  return ret{};
}
	
	template <typename L, typename R>
static auto _clear_empty_statements(Statement<Assignment<L, R>>)
{
  using newl = DECT(clear_empty_statements(L{}));
  using newr = DECT(clear_empty_statements(R{}));
  struct ret
  {
    using ast = Statement<Assignment<typename newl::ast, typename newr::ast>>;
    using remove_from_require = DECT(newl::remove_from_require::combine(typename newr::remove_from_require{}));
    using still_require = DECT(newl::still_require::combine(typename newr::still_require{}));
  };
  return ret{};
}
	
template <typename R>
static auto _clear_empty_statements(Statement<Return<R>>)
{
  using newr = DECT(clear_empty_statements(R{}));
  struct ret
  {
    using ast = Statement<Return<typename newr::ast>>;
    using remove_from_require = typename newr::remove_from_require;
    using still_require = typename newr::still_require;
  };
  return ret{};
}

template <typename R>
static auto _clear_empty_statements(Statement<AccompanyWrite<R>>)
{
  using newr = DECT(clear_empty_statements(R{}));
  struct ret
  {
    using ast = Statement<AccompanyWrite<typename newr::ast>>;
    using remove_from_require = typename newr::remove_from_require;
    using still_require = typename newr::still_require;
  };
  return ret{};
}

template <char... var>
static auto _clear_empty_statements(Statement<IncrementOccurance<String<var...>>> a)
{
  struct ret
  {
    using ast = DECT(a);
    using remove_from_require = mutils::typeset<>;
    using still_require = mutils::typeset<>;
  };
  return ret{};
}

  template <typename T>
static auto _clear_empty_statements(Statement<WriteTombstone<T> >)
{
  using newr = DECT(clear_empty_statements(T{}));
  struct ret
  {
    using ast = Statement<WriteTombstone<typename newr::ast>>;
    using remove_from_require = typename newr::remove_from_require;
    using still_require = typename newr::still_require;
  };
  return ret{};
}

template <char... var>
static auto _clear_empty_statements(Statement<IncrementRemoteOccurance<String<var...>>> a)
{
  struct ret
  {
    using ast = DECT(a);
    using remove_from_require = mutils::typeset<>;
	  //because we don't know what this variable is aliasing,
	  //we have to consider it still required;
	  //even if its only appearance in this phase is here, some
	  //other used + aliased variable might exist.
	  using still_require = mutils::typeset<String<var...>>;
  };
  return ret{};
}

	template <typename hndl_t, char... var>
static auto _clear_empty_statements(Statement<IncrementOccurance<Expression<hndl_t,VarReference<String<var...> > > > > a)
{
  struct ret
  {
    using ast = DECT(a);
    using remove_from_require = mutils::typeset<>;
	  //because we don't know what this variable is aliasing,
	  //we have to consider it still required;
	  //even if its only appearance in this phase is here, some
	  //other used + aliased variable might exist.
	  using still_require = mutils::typeset<String<var...> >;
  };
  return ret{};
}

	template <typename hndl_t, char... var>
static auto _clear_empty_statements(Statement<RefreshRemoteOccurance<Expression<hndl_t,VarReference<String<var...> > > > > a)
{
  struct ret
  {
    using ast = DECT(a);
    using remove_from_require = mutils::typeset<>;
	  //because we don't know what this variable is aliasing,
	  //we have to consider it still required;
	  //even if its only appearance in this phase is here, some
	  //other used + aliased variable might exist.
	  using still_require = mutils::typeset<String<var...> >;
  };
  return ret{};
}

template <typename c, typename t, typename e>
static auto _clear_empty_statements(Statement<If<c, t, e>>)
{
  using newc = DECT(clear_empty_statements(c{}));
  using newt = DECT(clear_empty_statements(t{}));
  using newe = DECT(clear_empty_statements(e{}));
  constexpr bool empty_if = is_empty_sequence<typename newt::ast>::value && is_empty_sequence<typename newe::ast>::value;
  using stmt = std::conditional_t<empty_if, Sequence<>,
                                  If<typename newc::ast, typename newt::ast, typename newe::ast>>;
  struct ret
  {
    using ast = Statement<stmt>;
    using remove_from_require =
      std::conditional_t<empty_if, typename newc::still_require,
                         DECT(newc::remove_from_require::combine(typename newt::remove_from_require{}).combine(typename newe::remove_from_require{}))>;
    using still_require = std::conditional_t<empty_if, mutils::typeset<>,
                                             DECT(newc::still_require::combine(typename newt::still_require{}).combine(typename newe::still_require{}))>;
  };
  return ret{};
}

template <typename c, typename t, char... name>
static auto _clear_empty_statements(Statement<While<c, t, name...>>)
{
  using newc = DECT(clear_empty_statements(c{}));
  using newt = DECT(clear_empty_statements(t{}));

  struct ret
  {
    using ast = Statement<While<typename newc::ast, typename newt::ast, name...>>;
    using remove_from_require = DECT(newc::remove_from_require::combine(typename newt::remove_from_require{}));
    using still_require = DECT(newc::still_require::combine(typename newt::still_require{}));
  };
  return ret{};
}

template <typename t, char... name>
static auto _clear_empty_statements(Statement<ForEach<t, name...>>)
{
  using newt = DECT(clear_empty_statements(t{}));
  struct ret
  {
    using ast = Statement<ForEach<typename newt::ast, name...>>;
    using remove_from_require = typename newt::remove_from_require;
    using still_require = typename newt::still_require;
  };
  return ret{};
}

template <typename... Seq>
static auto _clear_empty_statements(Statement<Sequence<Seq...>>)
{
  using namespace mutils;
  struct ret
  {
    using ast = DECT(collapse(Statement<Sequence<typename DECT(clear_empty_statements(Seq{}))::ast...>>{}));
    using remove_from_require = DECT(typelist_ns::combine(typename DECT(clear_empty_statements(Seq{}))::remove_from_require{}...));
    using still_require = DECT(typelist_ns::combine(typename DECT(clear_empty_statements(Seq{}))::still_require{}...));
  };
  return ret{};
}

	END_SPLIT_CONTEXT;

	template<typename l>
	template <typename AST>
	auto remove_empties<l>::clear_empty_statements(AST a)
	{
		return _clear_empty_statements(a);
	}

	template<typename l, typename AST>
	auto clear_empty_statements(AST a){
		return remove_empties<l>::clear_empty_statements(a);
	}
}
}
}
