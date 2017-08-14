#pragma once

namespace myria {
namespace mtl {
namespace split_phase {

template <typename Name, typename... bindings>
constexpr bool contains_var_of_name(Name, mutils::typeset<bindings...>)
{
  return mutils::contains_match<DECT(bindings::get_binding2(Name{}))...>();
}

template <typename label, typename AST, typename... reqs>
constexpr auto remove_unused(AST a, mutils::typeset<reqs...> ts);

template <typename l, typename typeset, typename b, typename body>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template Let<b, body>>, typeset)
{
  return typename AST<l>::template Statement<typename AST<l>::template Let<b, DECT(remove_unused<l>(body{}, typeset{}))>>{};
}

template <typename l, typename typeset, typename b, typename body>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template LetRemote<b, body>>, typeset)
{
  return typename AST<l>::template Statement<typename AST<l>::template LetRemote<b, DECT(remove_unused<l>(body{}, typeset{}))>>{};
}

	template <typename l, typename typeset, typename b, typename h, typename body>
	constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template LetIsValid<b, h, body>>, typeset)
{
  return typename AST<l>::template Statement<typename AST<l>::template LetIsValid<b, h, DECT(remove_unused<l>(body{}, typeset{}))>>{};
}

	template <typename l, typename typeset, typename n, typename h, typename... a>
	constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template StatementOperation<n, h, a...>> _a, typeset)
{
	return _a;
}

template <typename l, typename typeset, typename L, typename R>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template Assignment<L, R>> a, typeset)
{
  return a;
}

template <typename l, typename typeset, typename R>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template Return<R>> a, typeset)
{
  return a;
}

template <typename l, typename typeset, typename R>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template AccompanyWrite<R>> a, typeset)
{
  return a;
}

  template <typename l, typename typeset, typename R>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template WriteTombstone<R>> a, typeset)
{
  return a;
}

template <typename l, typename typeset, char... var>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template IncrementOccurance<String<var...>>>, typeset ts)
{
  return typename AST<l>::template Statement<std::conditional_t<
    contains_var_of_name(String<var...>{}, ts), typename AST<l>::template IncrementOccurance<String<var...>>, typename AST<l>::template Sequence<>>>{};
}

template <typename l, typename typeset, char... var>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template IncrementRemoteOccurance<String<var...>>>, typeset ts)
{
  return typename AST<l>::template Statement<std::conditional_t<
    contains_var_of_name(String<var...>{}, ts), typename AST<l>::template IncrementRemoteOccurance<String<var...>>, typename AST<l>::template Sequence<>>>{};
}

template <typename l, typename typeset, typename hndl_t, char... var>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template IncrementOccurance<typename AST<l>::template Expression<hndl_t,typename AST<l>::template VarReference<String<var...> > > > > a, typeset ts)
{
	return std::conditional_t<contains_var_of_name(String<var...>{}, ts), DECT(a), typename AST<l>::template Statement< typename AST<l>::template Sequence<> > >{};
}
template <typename l, typename typeset, typename hndl_t, char... var>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template RefreshRemoteOccurance<typename AST<l>::template Expression<hndl_t,typename AST<l>::template VarReference<String<var...> > > > > a, typeset ts)
{
	return std::conditional_t<contains_var_of_name(String<var...>{}, ts), DECT(a), typename AST<l>::template Statement< typename AST<l>::template Sequence<>>>{};
}

template <typename l, typename typeset, typename c, typename t, typename e>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template If<c, t, e>>, typeset ts)
{
  return typename AST<l>::template Statement<typename AST<l>::template If<c, DECT(remove_unused<l>(t{}, ts)), DECT(remove_unused<l>(e{}, ts))>>{};
}

template <typename l, typename typeset, typename c, typename t, char... name>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template While<c, t, name...>>, typeset ts)
{
  return typename AST<l>::template Statement<typename AST<l>::template While<c, DECT(remove_unused<l>(t{}, ts)), name...>>{};
}

template <typename l, typename typeset, typename t, char... name>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template ForEach<t, name...>>, typeset ts)
{
  return typename AST<l>::template Statement<typename AST<l>::template ForEach<DECT(remove_unused<l>(t{}, ts)), name...>>{};
}

template <typename l, typename typeset, typename... Seq>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template Sequence<Seq...>>, typeset ts)
{
  return typename AST<l>::template Statement<typename AST<l>::template Sequence<DECT(remove_unused<l>(Seq{}, ts))...>>{};
}

template <typename label, typename AST, typename... reqs>
constexpr auto remove_unused(AST a, mutils::typeset<reqs...> ts)
{
  return _remove_unused<label>(a, ts);
}

	template <typename _label, typename provides, typename inherits, typename _returns, typename _ast, typename... _requires>
	constexpr auto remove_unused(extracted_phase<Label<_label>, phase_api<Label<_label>, requires<_requires...>, provides, inherits>, _returns, _ast>)
{
  return extracted_phase<Label<_label>, phase_api<Label<_label>, requires<_requires...>, provides, inherits>,_returns,
                         DECT(remove_unused<Label<_label>>(_ast{}, mutils::typeset<>::template add<_requires...>()))>{};
}
}
}
}
