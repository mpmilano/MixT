#pragma once

namespace myria {
namespace mtl {
namespace split_phase {

template <typename Name, typename... bindings>
constexpr bool contains_var_of_name(Name, mutils::typeset<bindings...>)
{
  return mutils::contains_match<DECT(bindings::get_binding2(Name{}))...>();
}

	BEGIN_SPLIT_CONTEXT(ru);
	
template <typename AST, typename... reqs>
static constexpr auto remove_unused(AST a, mutils::typeset<reqs...> ts);

template < typename typeset, typename b, typename body>
static constexpr auto _remove_unused(const Statement<Let<b, body>>, typeset)
{
  return Statement<Let<b, DECT(remove_unused(body{}, typeset{}))>>{};
}

template < typename typeset, typename b, typename body>
static constexpr auto _remove_unused(const Statement<LetRemote<b, body>>, typeset)
{
  return Statement<LetRemote<b, DECT(remove_unused(body{}, typeset{}))>>{};
}

	template < typename typeset, typename n, typename h, typename... a>
	static constexpr auto _remove_unused(const Statement<Operation<n, h, a...>> _a, typeset)
{
	return _a;
}

template < typename typeset, typename L, typename R>
static constexpr auto _remove_unused(const Statement<Assignment<L, R>> a, typeset)
{
  return a;
}

template < typename typeset, typename R>
static constexpr auto _remove_unused(const Statement<Return<R>> a, typeset)
{
  return a;
}

template < typename typeset, typename R>
static constexpr auto _remove_unused(const Statement<AccompanyWrite<R>> a, typeset)
{
  return a;
}

  template < typename typeset, typename R>
static constexpr auto _remove_unused(const Statement<WriteTombstone<R>> a, typeset)
{
  return a;
}

template < typename typeset, char... var>
static constexpr auto _remove_unused(const Statement<IncrementOccurance<String<var...>>>, typeset ts)
{
  return Statement<std::conditional_t<
    contains_var_of_name(String<var...>{}, ts), IncrementOccurance<String<var...>>, Sequence<>>>{};
}

template < typename typeset, char... var>
static constexpr auto _remove_unused(const Statement<IncrementRemoteOccurance<String<var...>>>, typeset ts)
{
  return Statement<std::conditional_t<
    contains_var_of_name(String<var...>{}, ts), IncrementRemoteOccurance<String<var...>>, Sequence<>>>{};
}

template < typename typeset, typename hndl_t, char... var>
static constexpr auto _remove_unused(const Statement<IncrementOccurance<Expression<hndl_t,VarReference<String<var...> > > > > a, typeset ts)
{
	return std::conditional_t<contains_var_of_name(String<var...>{}, ts), DECT(a), Statement< Sequence<> > >{};
}
template < typename typeset, typename hndl_t, char... var>
static constexpr auto _remove_unused(const Statement<RefreshRemoteOccurance<Expression<hndl_t,VarReference<String<var...> > > > > a, typeset ts)
{
	return std::conditional_t<contains_var_of_name(String<var...>{}, ts), DECT(a), Statement< Sequence<>>>{};
}

template < typename typeset, typename c, typename t, typename e>
static constexpr auto _remove_unused(const Statement<If<c, t, e>>, typeset ts)
{
  return Statement<If<c, DECT(remove_unused(t{}, ts)), DECT(remove_unused(e{}, ts))>>{};
}

template < typename typeset, typename c, typename t, char... name>
static constexpr auto _remove_unused(const Statement<While<c, t, name...>>, typeset ts)
{
  return Statement<While<c, DECT(remove_unused(t{}, ts)), name...>>{};
}

template < typename typeset, typename t, char... name>
static constexpr auto _remove_unused(const Statement<ForEach<t, name...>>, typeset ts)
{
  return Statement<ForEach<DECT(remove_unused(t{}, ts)), name...>>{};
}

template < typename typeset, typename... Seq>
static constexpr auto _remove_unused(const Statement<Sequence<Seq...>>, typeset ts)
{
  return Statement<Sequence<DECT(remove_unused(Seq{}, ts))...>>{};
}

	END_SPLIT_CONTEXT;

	template <typename label>
	template <typename AST, typename... reqs>
	constexpr auto ru<label>::remove_unused(AST a, mutils::typeset<reqs...> ts)
	{
		return _remove_unused(a, ts);
	}

	template <typename _label, typename provides, typename inherits, typename _returns, typename _ast, typename... _requires>
	constexpr auto remove_unused(extracted_phase<Label<_label>, phase_api<Label<_label>, requires<_requires...>, provides, inherits>, _returns, _ast>)
{
  return extracted_phase<Label<_label>, phase_api<Label<_label>, requires<_requires...>, provides, inherits>,_returns,
                         DECT(ru<Label<_label>>::remove_unused(_ast{}, mutils::typeset<>::template add<_requires...>()))>{};
}
}
}
}
