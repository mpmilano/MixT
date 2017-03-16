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

template <typename l, typename typeset, typename L, typename R>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template Assignment<L, R>> a, typeset)
{
  return a;
}

template <typename l, typename typeset, char... var>
constexpr auto _remove_unused(const typename AST<l>::template Statement<typename AST<l>::template IncrementOccurance<String<var...>>>, typeset ts)
{
  return typename AST<l>::template Statement<std::conditional_t<
    contains_var_of_name(String<var...>{}, ts), typename AST<l>::template IncrementOccurance<String<var...>>, typename AST<l>::template Sequence<>>>{};
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

template <typename _label, typename provides, typename inherits, typename _ast, typename... _requires>
constexpr auto remove_unused(extracted_phase<Label<_label>, phase_api<Label<_label>, requires<_requires...>, provides, inherits>, _ast>)
{
  return extracted_phase<Label<_label>, phase_api<Label<_label>, requires<_requires...>, provides, inherits>,
                         DECT(remove_unused<Label<_label>>(_ast{}, mutils::typeset<>::template add<_requires...>()))>{};
}
}
}
}
