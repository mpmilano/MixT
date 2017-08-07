#pragma once
#include "writes_remote.hpp"

namespace myria {
namespace mtl {
namespace typecheck_phase {
namespace tracking_phase {

template <typename l, typename y, typename v, typename e>
constexpr auto extract_remote_binding(Binding<l, y, v, e>)
{
  return v{};
}

template <typename l, typename y, typename v, typename... remote_bound_things>
constexpr bool is_remote_bound(mutils::typeset<remote_bound_things...>, Expression<l, y, VarReference<v>>)
{
  return (std::is_same<typename v::name, typename remote_bound_things::name>::value || ... || false);
}

template <typename l, typename y, typename s, typename v, typename... remote_bound_things>
constexpr bool
is_remote_bound(mutils::typeset<remote_bound_things...> a, Expression<l, y, FieldReference<s, v>>)
{
  return is_remote_bound(a, s{});
}

template <typename tracked_labels, typename remote_bound, typename AST>
constexpr auto insert_tracking(AST a);

template <typename tracked_labels, typename remote_bound, typename l, typename Var, typename Expr>
constexpr auto _insert_tracking(Statement<l, Assignment<Var, Expr>> orig)
{
  return std::conditional_t < tracked_labels::strong::template contains<l>() && is_remote_bound(remote_bound{}, Var{}),
         Statement<l, Sequence<DECT(orig), Statement<l, AccompanyWrite<Var>>>>, DECT(orig) > {};
}

template <typename tracked_labels, typename remote_bound, typename l, typename Expr>
constexpr auto _insert_tracking(Statement<l, Return<Expr>> orig)
{
  return orig;
}

template <typename tracked_labels, typename remote_bound, typename l, typename b, typename e>
constexpr auto _insert_tracking(Statement<l, Let<b, e>>)
{
  return Statement<l, Let<b, DECT(insert_tracking<tracked_labels, remote_bound>(e{}))>>{};
}

template <typename tracked_labels, typename remote_bound, typename l, typename b, typename e>
constexpr auto _insert_tracking(Statement<l, LetRemote<b, e>>)
{
  return Statement<l, LetRemote<b, DECT(insert_tracking<tracked_labels, DECT(remote_bound::template add<DECT(extract_remote_binding(b{}))>())>(e{}))>>{};
}

template <typename tracked_labels, typename remote_bound, typename l, typename n, typename h, typename e>
constexpr auto _insert_tracking(Statement<l, LetIsValid<n,h, e>>)
{
  return Statement<l, LetIsValid<n,h, DECT(insert_tracking<tracked_labels, DECT(remote_bound::template add<n>())>(e{}))>>{};
}

template <typename tracked_labels, typename remote_bound, typename l, typename oper_name, typename Hndl, typename... args>
constexpr auto _insert_tracking(Statement<l, StatementOperation<oper_name, Hndl, args...>> a)
{
	return a;
}

template <typename tracked_labels, typename remote_bound, typename l, typename c, typename t, typename e>
constexpr auto _insert_tracking(Statement<l, If<c, t, e>>)
{
  return Statement<l, If<c, DECT(insert_tracking<tracked_labels, remote_bound>(t{})), DECT(insert_tracking<tracked_labels, remote_bound>(e{}))>>{};
}

template <typename tracked_labels, typename remote_bound, typename l, typename c, typename e>
constexpr auto _insert_tracking(Statement<l, While<c, e>>)
{
  return Statement<l, While<c, DECT(insert_tracking<tracked_labels, remote_bound>(e{}))>>{};
}

template <typename tracked_labels, typename remote_bound, typename l, typename... seq>
constexpr auto _insert_tracking(Statement<l, Sequence<seq...>>)
{
  return Statement<l, Sequence<DECT(insert_tracking<tracked_labels, remote_bound>(seq{}))...>>{};
}

template <typename tracked_labels, typename remote_bound, typename AST>
constexpr auto insert_tracking(AST a)
{
  return _insert_tracking<tracked_labels, remote_bound>(a);
}

template <typename, typename>
struct tracking_pair;
template <typename _strong_partner, typename... weak_elems>
struct tracking_pair<_strong_partner, mutils::typeset<weak_elems...>>
{
  using strong = _strong_partner;
  using weak = mutils::typeset<weak_elems...>;
  static constexpr strong strong_partner() { return strong{}; }
  static constexpr weak weak_partner() { return weak{}; }
	static constexpr auto combined() { return strong::combine(weak{});}
  constexpr tracking_pair() = default;
};

template <typename l1, typename l2>
constexpr auto needs_tracking(std::enable_if_t<l1::template tracks_against<l2>::value>* = nullptr)
{
  using namespace mutils;
  return tracking_pair<typeset<l2>, typeset<l1>>{};
}

template <typename l1, typename l2>
constexpr auto needs_tracking(std::enable_if_t<l2::template tracks_against<l1>::value>* = nullptr)
{
  using namespace mutils;
  return tracking_pair<typeset<l1>, typeset<l2>>{};
}

template <typename l1, typename l2>
constexpr auto needs_tracking(std::enable_if_t<!l2::template tracks_against<l1>::value && !l1::template tracks_against<l2>::value>* = nullptr)
{
  using namespace mutils;
  return tracking_pair<typeset<>, typeset<>>{};
}

constexpr auto tracked_labels(mutils::typelist<>)
{
  using namespace mutils;
  return tracking_pair<typeset<>, typeset<>>{};
}

template <typename l1, typename... labels>
constexpr auto tracked_labels(mutils::typelist<l1, labels...>)
{
  constexpr auto recr = tracked_labels(mutils::typelist<labels...>{});
  constexpr auto strong_labels = mutils::typeset<>::combine(needs_tracking<l1, labels>().strong_partner()...).combine(recr.strong_partner());
  constexpr auto weak_labels = mutils::typeset<>::combine(needs_tracking<l1, labels>().weak_partner()...).combine(recr.weak_partner());
  return tracking_pair<DECT(strong_labels), DECT(weak_labels)>{};
}

constexpr auto write_tombstones(mutils::typeset<>)
{
  return Sequence<>{};
}

template <typename l1, typename... labels>
constexpr auto write_tombstones(mutils::typeset<l1, labels...>)
{
  return Sequence<Statement<l1, WriteTombstone<Expression<Label<top>, tracker::Tombstone, VarReference<tombstone_str>>>>>::append(
    write_tombstones(mutils::typeset<labels...>{}));
}

	template <typename AST, typename labels, typename extra_strong_labels, typename extra_weak_labels>
	constexpr auto make_tracking_choice(AST a, std::true_type*, labels, extra_strong_labels, extra_weak_labels)
{
  // tracking is required
  constexpr auto collected = tracked_labels(labels{});
	using extended_collected = tracking_pair<DECT(collected.strong_partner().combine(extra_strong_labels{})),
																					 DECT(collected.weak_partner().combine(extra_weak_labels{}))>;
  using sorted = extended_collected;
  return Statement<Label<top>, Let<Binding<Label<top>, tracker::Tombstone, tombstone_str, Expression<Label<top>, tracker::Tombstone, GenerateTombstone>>,
                                   Statement<Label<top>, Sequence<Statement<Label<top>, DECT(write_tombstones(sorted::weak::combine(sorted::strong_partner())))>,
                                                                  DECT(insert_tracking<sorted, mutils::typeset<>>(a))>>>>{};
}

	template <typename AST, typename labels, typename ts1, typename ts2>
	constexpr auto make_tracking_choice(AST a, std::false_type*, labels, ts1, ts2)
{
  // no tracking is required on this transaction.
  return a;
}

constexpr bool any_pair_tracks(mutils::typelist<>)
{
  return false;
}

template <typename l1, typename... labels>
constexpr bool any_pair_tracks(mutils::typelist<l1, labels...>)
{
  return (l1::template tracks_against<labels>::value || ... || any_pair_tracks(mutils::typelist<labels...>{}));
}

// if we have *any* extra labels, then we assume that tracking will be enforced for this transaction.
template <typename AST, typename... extra_labels>
constexpr auto insert_tracking_begin(AST a)
{
  constexpr auto labels = collect_proper_labels(a).append(mutils::typelist<extra_labels...>{});
	constexpr bool writes_remote = begin_writes_remote(a);
  constexpr bool needs_tracking = writes_remote && (any_pair_tracks(labels) || any_pair_tracks(labels.reverse()) || (sizeof...(extra_labels) > 0));
  std::integral_constant<bool, needs_tracking>* choice{ nullptr };
  return make_tracking_choice(a, choice, labels,
															mutils::typeset<>::combine(std::conditional_t<!extra_labels::might_track::value, mutils::typeset<extra_labels>, mutils::typeset<> >{}...),
															mutils::typeset<>::combine(std::conditional_t<extra_labels::might_track::value, mutils::typeset<extra_labels>, mutils::typeset<> >{}...));
}
}
}
}

// take this transaction and add tombstone tracking for the relevant level as an extra thing.
template <typename previous_transaction_phases, typename... label>
constexpr auto tombstone_enhanced_txn_f(Label<label>...)
{
	using namespace mtl;
  using namespace typecheck_phase;
	using namespace tracking_phase;
  using transaction_rebuilder = previous_transaction_phases;
	using inferred = typename transaction_rebuilder::inferred;
	return typename transaction_rebuilder::template resume_compilation_inferred_str<
		DECT(insert_tracking_begin<inferred, Label<label>...>(inferred{}))>{};
}

template <typename previous_transaction_phases, typename... l>
using tombstone_enhanced_txn = typename DECT(tombstone_enhanced_txn_f<previous_transaction_phases>(l{}...))::recollapsed;
template <typename previous_transaction_phases, typename... l>
using tombstone_enhanced_store = typename DECT(tombstone_enhanced_txn_f<previous_transaction_phases>(l{}...))::all_store;
}
