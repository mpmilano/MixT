#pragma once
#include "AST_split.hpp"
#include "runnable_transaction.hpp"
#include "without_names.hpp"
#include <iostream>
#include <tuple>

namespace myria {
namespace mtl {
namespace split_phase {
namespace recollapse_ns {

using anorm_str = String<'a', 'n', 'o', 'r', 'm'>;

template <typename l, typename cand, typename AST>
constexpr bool contains_anorm(AST a);

template <typename l, typename cand, typename y, typename s, typename f>
constexpr bool _contains_anorm(typename AST<l>::template Expression<y, typename AST<l>::template FieldReference<s, f>>)
{
  return contains_anorm<l, cand>(s{});
}

template <typename l, typename cand, typename y, typename v>
constexpr bool _contains_anorm(typename AST<l>::template Expression<y, typename AST<l>::template VarReference<v>>)
{
  return cand::template contains<v>();
}

template <typename l, typename cand, long long i>
constexpr bool
_contains_anorm(typename AST<l>::template Expression<long long, typename AST<l>::template Constant<i>> a)
{
  return false;
}

template <typename l, typename cand, typename y, char op, typename L, typename R>
constexpr bool _contains_anorm(typename AST<l>::template Expression<y, typename AST<l>::template BinOp<op, L, R>>)
{
  return contains_anorm<l, cand>(L{}) || contains_anorm<l, cand>(R{});
}

template <typename l, typename cand, typename AST>
constexpr bool
contains_anorm(AST a)
{
  return _contains_anorm<l, cand>(a);
}

template <typename l, typename candidates, typename sub_map, typename AST>
auto recollapse(AST a);

template <typename l, typename candidates, typename sub_map, typename y, typename s, typename f>
auto _recollapse(typename AST<l>::template Expression<y, typename AST<l>::template FieldReference<s, f>>)
{
  recollapse<l, candidates, sub_map>(s{});
  return typename AST<l>::template Expression<y, typename AST<l>::template FieldReference<DECT(recollapse<l, candidates, sub_map>(s{})), f>>{};
}

template <typename l, typename candidates, typename sub_map, typename y, typename v>
auto _recollapse(typename AST<l>::template Expression<y, typename AST<l>::template VarReference<v>>,
                 std::enable_if_t<sub_map::template contains<v>()>* = nullptr)
{
  std::cout << "matched var reference : ";
  print_varname(std::cout, v{});
  std::cout << " " << sub_map{} << std::endl;
  return typename sub_map::template get<v>::value{};
}

template <typename l, typename candidates, typename sub_map, typename y, typename v>
auto _recollapse(typename AST<l>::template Expression<y, typename AST<l>::template VarReference<v>> a,
                 std::enable_if_t<!sub_map::template contains<v>()>* = nullptr)
{
  std::cout << "unmatched var reference : ";
  print_varname(std::cout, v{});
  std::cout << " " << sub_map{} << std::endl;
  return a;
}

template <typename l, typename candidates, typename sub_map, long long i>
auto _recollapse(typename AST<l>::template Expression<long long, typename AST<l>::template Constant<i>> a)
{
  return a;
}

template <typename l, typename candidates, typename sub_map, typename y, char op, typename L, typename R>
auto _recollapse(typename AST<l>::template Expression<y, typename AST<l>::template BinOp<op, L, R>>)
{
  recollapse<l, candidates, sub_map>(L{});
  recollapse<l, candidates, sub_map>(R{});
  using newl = DECT(recollapse<l, candidates, sub_map>(L{}));
  using newr = DECT(recollapse<l, candidates, sub_map>(R{}));
  return typename AST<l>::template Expression<y, typename AST<l>::template BinOp<op, newl, newr>>{};
}

template <typename l, typename candidates, typename sub_map, typename l2, typename y, typename v, typename e>
auto _recollapse(typename AST<l>::template Binding<l2, y, v, e>)
{
  std::cout << "submap: " << sub_map{} << std::endl;
  recollapse<l, candidates, sub_map>(e{});
  struct pack
  {
    using new_e = DECT(recollapse<l, candidates, sub_map>(e{}));
    // static_assert(!contains_anorm<l,candidates>(new_e{}));
    using new_map = std::conditional_t<candidates::template contains<v>(), typename sub_map::template add_unconditional<v, new_e>, sub_map>;
    using remove_this_binding = std::integral_constant<bool, candidates::template contains<v>()>;
    using binding = typename AST<l>::template Binding<l2, y, v, new_e>;
  };
  std::cout << "newmap: " << typename pack::new_map{} << std::endl;
  return pack{};
}

template <typename l, typename candidates, typename sub_map, typename b, typename body>
auto _recollapse(typename AST<l>::template Statement<typename AST<l>::template Let<b, body>>)
{

  recollapse<l, candidates, sub_map>(b{});
  using newb_trip = DECT(recollapse<l, candidates, sub_map>(b{}));
  using new_map = typename newb_trip::new_map;
  recollapse<l, candidates, new_map>(body{});
  std::cout << "bodymap: " << new_map{} << std::endl;
  using new_body = DECT(recollapse<l, candidates, new_map>(body{}));
  using newb = typename newb_trip::binding;
  return std::conditional_t<newb_trip::remove_this_binding::value, new_body,
                            typename AST<l>::template Statement<typename AST<l>::template Let<newb, new_body>>>{};
}

template <typename l, typename candidates, typename sub_map, typename b, typename body>
auto _recollapse(typename AST<l>::template Statement<typename AST<l>::template LetRemote<b, body>>)
{
  recollapse<l, candidates, sub_map>(b{});
  using newb = typename DECT(recollapse<l, candidates, sub_map>(b{}))::binding;
  recollapse<l, candidates, sub_map>(body{});
  using new_body = DECT(recollapse<l, candidates, sub_map>(body{}));
  return typename AST<l>::template Statement<typename AST<l>::template LetRemote<newb, new_body>>{};
}

template <typename l, typename candidates, typename sub_map, typename L, typename R>
auto _recollapse(typename AST<l>::template Statement<typename AST<l>::template Assignment<L, R>>)
{
  std::cout << "assignmap: " << sub_map{} << std::endl;
  recollapse<l, candidates, sub_map>(L{});
  recollapse<l, candidates, sub_map>(R{});
  return typename AST<l>::template Statement<
    typename AST<l>::template Assignment<DECT(recollapse<l, candidates, sub_map>(L{})), DECT(recollapse<l, candidates, sub_map>(R{}))>>{};
}

template <typename l, typename candidates, typename sub_map, char... var>
auto _recollapse(typename AST<l>::template Statement<typename AST<l>::template IncrementOccurance<String<var...>>> a)
{
  static_assert(!candidates::template contains<String<var...>>());
  return a;
}

template <typename l, typename candidates, typename sub_map, typename c, typename t, typename e>
auto _recollapse(typename AST<l>::template Statement<typename AST<l>::template If<c, t, e>>)
{
  recollapse<l, candidates, sub_map>(c{});
  recollapse<l, candidates, sub_map>(t{});
  recollapse<l, candidates, sub_map>(e{});
  return typename AST<l>::template Statement<typename AST<l>::template If<
    DECT(recollapse<l, candidates, sub_map>(c{})), DECT(recollapse<l, candidates, sub_map>(t{})), DECT(recollapse<l, candidates, sub_map>(e{}))>>{};
}

template <typename l, typename candidates, typename sub_map, typename c, typename t, char... name>
auto _recollapse(typename AST<l>::template Statement<typename AST<l>::template While<c, t, name...>>)
{
  recollapse<l, candidates, sub_map>(c{});
  recollapse<l, candidates, sub_map>(t{});
  return typename AST<l>::template Statement<
    typename AST<l>::template While<DECT(recollapse<l, candidates, sub_map>(c{})), DECT(recollapse<l, candidates, sub_map>(t{})), name...>>{};
}

template <typename l, typename candidates, typename sub_map, typename t, char... name>
auto _recollapse(typename AST<l>::template Statement<typename AST<l>::template ForEach<t, name...>>)
{
  recollapse<l, candidates, sub_map>(t{});
  return typename AST<l>::template Statement<typename AST<l>::template ForEach<DECT(recollapse<l, candidates, sub_map>(t{})), name...>>{};
}

template <typename l, typename candidates, typename sub_map, typename... Seq>
auto _recollapse(typename AST<l>::template Statement<typename AST<l>::template Sequence<Seq...>>)
{
  std::make_tuple(recollapse<l, candidates, sub_map>(Seq{})...);
  return typename AST<l>::template Statement<typename AST<l>::template Sequence<DECT(recollapse<l, candidates, sub_map>(Seq{}))...>>{};
}

template <typename l, typename candidates, typename sub_map, typename AST>
auto recollapse(AST a)
{
  return _recollapse<l, candidates, sub_map>(a);
}

template <typename accum>
constexpr auto anorm_names2(accum a, mutils::typeset<>)
{
  return a;
}

template <typename accum, typename name1, typename... names>
constexpr auto anorm_names2(accum, mutils::typeset<name1, names...>, std::enable_if_t<!name1::begins_with(anorm_str{})>* = nullptr);

template <typename accum, typename name1, typename... names>
constexpr auto anorm_names2(accum, mutils::typeset<name1, names...>, std::enable_if_t<name1::begins_with(anorm_str{})>* = nullptr)
{
  using namespace mutils;
  return anorm_names2(accum::append(typelist<name1>{}), typeset<names...>{});
}

template <typename accum, typename name1, typename... names>
constexpr auto anorm_names2(accum, mutils::typeset<name1, names...>, std::enable_if_t<!name1::begins_with(anorm_str{})>*)
{
  using namespace mutils;
  return anorm_names2(accum{}, typeset<names...>{});
}

template <typename... vars>
constexpr auto anorm_names(mutils::typeset<vars...>)
{
  using namespace mutils;
  return anorm_names2(typelist<>{}, typeset<typename vars::name...>{});
}

template <typename l, typename AST, typename reqs, typename provides, typename owns, typename passthrough>
constexpr auto recollapse_phase(runnable_transaction::phase<l, AST, reqs, provides, owns, passthrough>)
{
  using namespace runnable_transaction;
  using namespace mutils;
  anorm_names(owns{});
  using names = DECT(anorm_names(owns{}));
  recollapse<l, names, type_association_map<>>(AST{});
  using new_ast = DECT(recollapse<l, names, type_association_map<>>(AST{}));
  std::cout << "GREP THIS " << names{} << std::endl;
  return phase<l, new_ast, reqs, provides, DECT(without_names(names{}, owns{})), passthrough>{};
}
}

template <typename... phases>
constexpr auto recollapse(runnable_transaction::transaction<phases...>)
{
  using namespace runnable_transaction;
  using namespace recollapse_ns;
  return transaction<DECT(recollapse_phase(phases{}))...>{};
}
}
}
}
