#pragma once

namespace myria { namespace mtl  { namespace typecheck_phase { namespace tracking_phase {

	template<typename l, typename y, typename v, typename e>
	constexpr auto extract_remote_binding(Binding<l,y,v,e>){
	  return v{};
	}
	
	template<typename l, typename y, typename v, typename... remote_bound_things>
	constexpr bool is_remote_bound(mutils::typeset<remote_bound_things...>, Expression<l,y,VarReference<v> >){
	  return (std::is_same<typename v::name,typename remote_bound_things::name>::value || ... || false);
	}

	template<typename l, typename y, typename s, typename v, typename... remote_bound_things>
	constexpr bool is_remote_bound(mutils::typeset<remote_bound_things...> a, Expression<l,y,FieldReference<s,v> >){
	  return is_remote_bound(a, s{});
	}

	template <typename tracked_labels, typename remote_bound, typename AST>
	constexpr auto insert_tracking(AST a);
	
	template <typename tracked_labels, typename remote_bound, typename l, typename Var, typename Expr>
	constexpr auto _insert_tracking(Statement<l, Assignment<Var, Expr>> orig)
	{
	  return std::conditional_t<
	    tracked_labels::strong::template contains<l>() && is_remote_bound(remote_bound{}, Var{}),
	    Statement<l,Sequence<DECT(orig), Statement<l,AccompanyWrite<Var> > > >,
	    DECT(orig) >{};
	}

	template <typename tracked_labels, typename remote_bound, typename l, typename Expr>
	constexpr auto _insert_tracking(Statement<l, Return<Expr>> orig)
	{
		return orig;
	}
	
	template <typename tracked_labels, typename remote_bound, typename l, typename b, typename e>
	constexpr auto _insert_tracking(Statement<l, Let<b, e>>)
	{
	  return Statement<l,Let<b,DECT(insert_tracking<tracked_labels, remote_bound>(e{})) > >{};
	}

	template <typename tracked_labels, typename remote_bound, typename l, typename b, typename e>
	constexpr auto _insert_tracking(Statement<l, LetRemote<b, e>>)
	{
	  return Statement<l,LetRemote<b,DECT(insert_tracking<tracked_labels,
																				DECT(remote_bound::template add<DECT(extract_remote_binding(b{}))>())>(e{})) > >{};
	}
	
	template <typename tracked_labels, typename remote_bound, typename l, typename c, typename t, typename e>
	constexpr auto _insert_tracking(Statement<l, If<c, t, e>>)
	{
	  return Statement<l,If<c,
				DECT(insert_tracking<tracked_labels,remote_bound>(t{})),
				DECT(insert_tracking<tracked_labels,remote_bound>(e{})) > >{};
	}
	
	template <typename tracked_labels, typename remote_bound, typename l, typename c, typename e>
	constexpr auto _insert_tracking(Statement<l, While<c, e>>)
	{
	  return Statement<l,While<c,DECT(insert_tracking<tracked_labels,remote_bound>(e{}))> >{};
	}
	
	template <typename tracked_labels, typename remote_bound, typename l, typename... seq>
	constexpr auto _insert_tracking(Statement<l, Sequence<seq...>>)
	{
	  return Statement<l,Sequence<DECT(insert_tracking<tracked_labels,remote_bound>(seq{}))...> >{};
	}
	
	
	template <typename tracked_labels, typename remote_bound, typename AST>
	constexpr auto insert_tracking(AST a)
	{
	  return _insert_tracking<tracked_labels,remote_bound>(a);
	}

	template<typename _strong_partner, typename _weak_partner>
	struct tracking_pair{
	  using strong = _strong_partner;
	  using weak = _weak_partner;
	  static constexpr strong strong_partner(){return strong{};}
	  static constexpr weak weak_partner(){return weak{};}
	  constexpr tracking_pair() = default;
	};

	template<typename l1, typename l2>
	constexpr auto needs_tracking(std::enable_if_t<l1::template tracks_against<l2>::value>* = nullptr){
	  using namespace mutils;
	  return tracking_pair<typeset<l2>,typeset<l1> >{};
	}

	template<typename l1, typename l2>
	constexpr auto needs_tracking(std::enable_if_t<l2::template tracks_against<l1>::value>* = nullptr){
	  using namespace mutils;
	  return tracking_pair<typeset<l1>,typeset<l2> >{};
	}

	template<typename l1, typename l2>
	constexpr auto needs_tracking(std::enable_if_t<!l2::template tracks_against<l1>::value && !l1::template tracks_against<l2>::value>* = nullptr){
	  using namespace mutils;
	  return tracking_pair<typeset<>,typeset<> >{};
	}

	template<typename l1, typename... labels>
	constexpr auto tracked_labels(mutils::typelist<l1,labels...>){
	  constexpr auto strong_labels = mutils::typeset<>::combine(needs_tracking<l1,labels>().strong_partner()...);
	  constexpr auto weak_labels = mutils::typeset<>::combine(needs_tracking<l1,labels>().weak_partner()...);
	  return tracking_pair<DECT(strong_labels),DECT(weak_labels)>{};
	}

	constexpr auto write_tombstones(mutils::typelist<>){
	  return Sequence<>{};
	}
	
	template<typename l1, typename... labels>
	constexpr auto write_tombstones(mutils::typelist<l1,labels...>){
	  return Sequence<Statement<l1,WriteTombstone> >::append(write_tombstones(mutils::typelist<labels...>{}));
	}

	template<typename AST, typename labels>
	constexpr auto make_tracking_choice(AST a, std::true_type*, labels){
	  //tracking is required
	  constexpr auto collected = tracked_labels(labels{});
	  using sorted = tracking_pair<DECT(sort_labels(collected.strong_partner())),DECT(sort_labels(collected.weak_partner()))>;
	  return Statement<Label<top>,Let<
					Binding<Label<top>,tracker::Tombstone,tombstone_str, Expression<Label<top>, tracker::Tombstone, GenerateTombstone> >,
					Statement<Label<top>,Sequence<
					   DECT(insert_tracking<sorted,mutils::typeset<> >(a)),
					   Statement<Label<top>, DECT(write_tombstones(typename sorted::weak{})) >
							       > > > >{};
	}
	
	template<typename AST, typename labels>
	constexpr auto make_tracking_choice(AST a, std::false_type*, labels){
	  //no tracking is required on this transaction.
	  return a;
	}

	constexpr bool any_pair_tracks(mutils::typelist<>){
	  return false;
	}

	template<typename l1, typename... labels> constexpr bool any_pair_tracks(mutils::typelist<l1,labels...>){
	  return (l1::template tracks_against<labels>::value || ... || any_pair_tracks(mutils::typelist<labels...>{}));
	}

	template <typename AST>
	constexpr auto insert_tracking_begin(AST a){
	  constexpr auto labels = collect_proper_labels(a);
	  constexpr bool needs_tracking = any_pair_tracks(labels) || any_pair_tracks(labels.reverse());
	  std::integral_constant<bool, needs_tracking>* choice{nullptr};
	  return make_tracking_choice(a,choice,labels);
	}

       }}}}
