#pragma once
#include "mtl/AST_split.hpp"
#include "mtl/runnable_transaction.hpp"
#include "mtl/without_names.hpp"

namespace myria {
namespace mtl {
namespace split_phase {
namespace recollapse_ns {

using anorm_str = String<'a', 'n', 'o', 'r', 'm'>;


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

	BEGIN_SPLIT_CONTEXT(recollapse_ctx);
	
template <typename cand, typename AST>
static constexpr bool contains_anorm(AST a);

template <typename cand, typename y, typename s, typename f>
static constexpr bool _contains_anorm(Expression<y, FieldReference<s, f>>)
{
  return contains_anorm< cand>(s{});
}

template <typename cand, typename y, typename v>
static constexpr bool _contains_anorm(Expression<y, VarReference<v>>)
{
  return cand::template contains<v>();
}

template <typename cand, int i>
static constexpr bool
_contains_anorm(Expression<int, Constant<i>> )
{
  return false;
}

template <typename cand>
static constexpr bool
_contains_anorm(Expression<tracker::Tombstone, GenerateTombstone> )
{
  return false;
}

template <typename cand, typename y, char op, typename L, typename R>
static constexpr bool _contains_anorm(Expression<y, BinOp<op, L, R>>)
{
  return contains_anorm< cand>(L{}) || contains_anorm< cand>(R{});
}

template <typename candidates, typename sub_map, typename AST>
static auto recollapse(AST a);

template <typename candidates, typename sub_map, typename y, typename s, typename f>
static auto _recollapse(Expression<y, FieldReference<s, f>>)
{
  return Expression<y, FieldReference<DECT(recollapse< candidates, sub_map>(s{})), f>>{};
}

template <typename candidates, typename sub_map, typename y, typename v>
static auto _recollapse(Expression<y, VarReference<v>>,
                 std::enable_if_t<sub_map::template contains<v>()>* = nullptr)
{
  return typename sub_map::template get<v>::value{};
}

template <typename candidates, typename sub_map, typename y, typename v>
static auto _recollapse(Expression<y, VarReference<v>> a,
                 std::enable_if_t<!sub_map::template contains<v>()>* = nullptr)
{
  return a;
}

template <typename candidates, typename sub_map, int i>
static auto _recollapse(Expression<int, Constant<i>> a)
{
  return a;
}

template <typename candidates, typename sub_map>
static auto _recollapse(Expression<tracker::Tombstone, GenerateTombstone> a)
{
  return a;
}

template <typename candidates, typename sub_map, typename y, char op, typename L, typename R>
static auto _recollapse(Expression<y, BinOp<op, L, R>>)
{
  using newl = DECT(recollapse< candidates, sub_map>(L{}));
  using newr = DECT(recollapse< candidates, sub_map>(R{}));
  return Expression<y, BinOp<op, newl, newr>>{};
}
	template <typename candidates, typename sub_map, typename y, typename h>
	static auto _recollapse(Expression<y,IsValid<h>>)
{
	return Expression<y,IsValid<DECT(recollapse<candidates,sub_map>(h{}))>>{};
}

	template <typename candidates, typename sub_map, typename n, typename h, typename... a>
	static auto _recollapse(Operation<n,h,a...>)
{
	return Operation<n,DECT(recollapse<candidates,sub_map>(h{})),DECT(recollapse<candidates,sub_map>(a{}))...>{};
}

	template <typename candidates, typename sub_map, typename n, typename h, typename... a>
	static auto _recollapse(Statement<Operation<n,h,a...>>)
{
	return Statement<DECT(_recollapse<candidates,sub_map>(Operation<n,h,a...>{})) >{};
}

	template <typename candidates, typename sub_map, typename y, typename n, typename h, typename... a>
	static auto _recollapse(Expression<y,Operation<n,h,a...>>)
{
	return Expression<y,DECT(_recollapse<candidates,sub_map>(Operation<n,h,a...>{})) >{};
}


  template<typename _new_e, typename _new_map, typename _remove_this_binding, typename _binding>
  struct recollapse_pack
  {
    using new_e = _new_e;
    using new_map = _new_map;
    using remove_this_binding = _remove_this_binding;
    using binding = _binding;
  };
  
template <typename candidates, typename sub_map, typename l2, typename y, typename v, typename e>
static auto _recollapse(Binding<l2, y, v, e>)
{
  struct pack
  {
    using new_e = DECT(recollapse< candidates, sub_map>(e{}));
    // static_assert(!contains_anorm<candidates>(new_e{}));
    using new_map = std::conditional_t<candidates::template contains<v>(), typename sub_map::template add_unconditional<v, new_e>, sub_map>;
    using remove_this_binding = std::integral_constant<bool, candidates::template contains<v>()>;
    using binding = Binding<l2, y, v, new_e>;
  };
  return recollapse_pack<typename pack::new_e, typename pack::new_map, typename pack::remove_this_binding, typename pack::binding>{};
}

template <typename candidates, typename sub_map, typename b, typename body>
static auto _recollapse(Statement<Let<b, body>>)
{

  using newb_trip = DECT(recollapse< candidates, sub_map>(b{}));
  using new_map = typename newb_trip::new_map;
  using new_body = DECT(recollapse< candidates, new_map>(body{}));
  using newb = typename newb_trip::binding;
  return std::conditional_t<newb_trip::remove_this_binding::value, new_body,
                            Statement<Let<newb, new_body>>>{};
}

template <typename candidates, typename sub_map, typename b, typename body>
static auto _recollapse(Statement<LetRemote<b, body>>)
{
  using newb = typename DECT(recollapse< candidates, sub_map>(b{}))::binding;
  using new_body = DECT(recollapse< candidates, sub_map>(body{}));
  return Statement<LetRemote<newb, new_body>>{};
}

template <typename candidates, typename sub_map, typename L, typename R>
static auto _recollapse(Statement<Assignment<L, R>>)
{
  return Statement<
    Assignment<DECT(recollapse< candidates, sub_map>(L{})), DECT(recollapse< candidates, sub_map>(R{}))>>{};
}

template <typename candidates, typename sub_map, typename R>
static auto _recollapse(Statement<Return<R>>)
{
  return Statement<
    Return<DECT(recollapse< candidates, sub_map>(R{}))>>{};
}

template <typename candidates, typename sub_map, typename R>
static auto _recollapse(Statement<AccompanyWrite<R>>)
{
  return Statement<
    AccompanyWrite<DECT(recollapse< candidates, sub_map>(R{}))>>{};
}

template <typename candidates, typename sub_map, typename R>
static auto _recollapse(Statement<WriteTombstone<R>>)
{
  return Statement<
    WriteTombstone<DECT(recollapse< candidates, sub_map>(R{}))>>{};
}

template <typename candidates, typename sub_map, char... var>
static auto _recollapse(Statement<IncrementOccurance<String<var...>>> a)
{
  static_assert(!candidates::template contains<String<var...>>());
  return a;
}

template <typename candidates, typename sub_map, char... var>
static auto _recollapse(Statement<IncrementRemoteOccurance<String<var...>>> a)
{
  static_assert(!candidates::template contains<String<var...>>());
  return a;
}

template <typename candidates, typename sub_map, typename hndl_t, char... var>
static auto _recollapse(Statement<IncrementOccurance<Expression<hndl_t,VarReference<String<var...> > > > > a)
{
	static_assert(!candidates::template contains<String<var...>>());
	return a;
}

template <typename candidates, typename sub_map, typename hndl_t, char... var>
static auto _recollapse(Statement<RefreshRemoteOccurance<Expression<hndl_t,VarReference<String<var...> > > > > a)
{
	static_assert(!candidates::template contains<String<var...>>());
	return a;
}

template <typename candidates, typename sub_map, typename c, typename t, typename e>
static auto _recollapse(Statement<If<c, t, e>>)
{
  return Statement<If<
    DECT(recollapse< candidates, sub_map>(c{})), DECT(recollapse< candidates, sub_map>(t{})), DECT(recollapse< candidates, sub_map>(e{}))>>{};
}

template <typename candidates, typename sub_map, typename c, typename t, char... name>
static auto _recollapse(Statement<While<c, t, name...>>)
{
  return Statement<
    While<DECT(recollapse< candidates, sub_map>(c{})), DECT(recollapse< candidates, sub_map>(t{})), name...>>{};
}

template <typename candidates, typename sub_map, typename t, char... name>
static auto _recollapse(Statement<ForEach<t, name...>>)
{
  return Statement<ForEach<DECT(recollapse< candidates, sub_map>(t{})), name...>>{};
}

template <typename candidates, typename sub_map, typename... Seq>
static auto _recollapse(Statement<Sequence<Seq...>>)
{
	return Statement<Sequence<DECT(recollapse< candidates, sub_map>(Seq{}))...>>{};
}

	END_SPLIT_CONTEXT;
	
template <typename l>
template <typename candidates, typename sub_map, typename AST>
auto recollapse_ctx<l>::recollapse(AST a)
{
  return _recollapse<candidates, sub_map>(a);
}
	template <typename l>
template <typename cand, typename AST>
constexpr bool recollapse_ctx<l>::contains_anorm(AST a)
{
  return _contains_anorm<cand>(a);
}

	
	template <typename l, typename returns, typename AST, typename reqs, typename provides, typename owns, typename passthrough>
	constexpr auto recollapse_phase(runnable_transaction::phase<l, returns, AST, reqs, provides, owns, passthrough>)
{
  using namespace runnable_transaction;
  using namespace mutils;
  using names = DECT(anorm_names(owns{}));
  using new_ast = DECT(recollapse_ctx<l>::template recollapse<names, type_association_map<>>(AST{}));
  return phase<l, returns, new_ast, reqs, provides, DECT(without_names(names{}, owns{})), passthrough>{};
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
