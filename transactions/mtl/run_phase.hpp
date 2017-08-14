#pragma once
#include "runnable_transaction.hpp"
#include "phase_context.hpp"
#include "AST_typecheck.hpp"
#include <thread>

namespace myria {
namespace mtl {
namespace runnable_transaction {

template <typename>
struct ReturnedValue;
template <>
struct ReturnedValue<void>
{
};
template <typename T>
struct ReturnedValue
{
  T value;
};

template <typename l, typename AST, typename TranCtx, typename store>
auto run_phase(AST*, TranCtx&, store& s);

template <typename l, typename TranCtx, typename store, typename y, typename str, typename f>
auto _run_phase(typename AST<l>::template Expression<y, typename AST<l>::template FieldReference<str, f>>*, TranCtx& ctx, store& s)
{
  constexpr str* next{ nullptr };
  return run_phase<l>(next, ctx, s).field(f{});
}

template <typename l, typename TranCtx, typename store, typename y, typename v>
auto _run_phase(typename AST<l>::template Expression<y, typename AST<l>::template VarReference<v>>*, TranCtx& ctx, store& s)
{
	constexpr auto explicit_v = v{};
	return s.get(explicit_v).get(s,ctx);
}

template <typename l, typename TranCtx, typename store, int i>
auto _run_phase(typename AST<l>::template Expression<int, typename AST<l>::template Constant<i>>*, TranCtx&, store&)
{
  return i;
}

template <typename l, typename TranCtx, typename store, typename y, typename L, typename R>
auto _run_phase(typename AST<l>::template Expression<y, typename AST<l>::template BinOp<'+', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase<l>(nextl, ctx, s) + run_phase<l>(nextR, ctx, s);
}

template <typename l, typename TranCtx, typename store, typename y, typename L, typename R>
auto _run_phase(typename AST<l>::template Expression<y, typename AST<l>::template BinOp<'-', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase<l>(nextl, ctx, s) - run_phase<l>(nextR, ctx, s);
}

template <typename l, typename TranCtx, typename store, typename y, typename L, typename R>
auto _run_phase(typename AST<l>::template Expression<y, typename AST<l>::template BinOp<'*', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase<l>(nextl, ctx, s) * run_phase<l>(nextR, ctx, s);
}

template <typename l, typename TranCtx, typename store, typename y, typename L, typename R>
auto _run_phase(typename AST<l>::template Expression<y, typename AST<l>::template BinOp<'/', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase<l>(nextl, ctx, s) / run_phase<l>(nextR, ctx, s);
}

template <typename l, typename TranCtx, typename store, typename y, typename L, typename R>
auto _run_phase(typename AST<l>::template Expression<y, typename AST<l>::template BinOp<'=', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase<l>(nextl, ctx, s) == run_phase<l>(nextR, ctx, s);
}

template <typename l, typename TranCtx, typename store, typename y, typename L, typename R>
auto _run_phase(typename AST<l>::template Expression<y, typename AST<l>::template BinOp<'&', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase<l>(nextl, ctx, s) && run_phase<l>(nextR, ctx, s);
}

template <typename l, typename TranCtx, typename store, typename y, typename L, typename R>
auto _run_phase(typename AST<l>::template Expression<y, typename AST<l>::template BinOp<'|', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase<l>(nextl, ctx, s) || run_phase<l>(nextR, ctx, s);
}

template <typename l, typename TranCtx, typename store, typename y, typename L, typename R>
auto _run_phase(typename AST<l>::template Expression<y, typename AST<l>::template BinOp<'>', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase<l>(nextl, ctx, s) > run_phase<l>(nextR, ctx, s);
}

template <typename l, typename TranCtx, typename store, typename y, typename L, typename R>
auto _run_phase(typename AST<l>::template Expression<y, typename AST<l>::template BinOp<'<', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  auto lval = run_phase<l>(nextl, ctx, s);
  auto rval = run_phase<l>(nextR, ctx, s);
  return lval < rval;
}

template <typename l, typename TranCtx, typename store, typename label, typename Yield, typename Expr, char... name>
auto _run_phase(typename AST<l>::template Binding<Label<label>, Yield, String<name...>, Expr>*, TranCtx& ctx, store& s)
{
  constexpr Expr* expr{ nullptr };
  constexpr String<name...> varname{};
  (void)varname;
  auto expr_result = run_phase<l>(expr, ctx, s);
  s.get(String<name...>{}).bind(s,ctx, expr_result);
}

template <typename l, typename TranCtx, typename store, typename Binding, typename Body>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template Let<Binding, Body>>*, TranCtx& ctx, store& s)
{
  constexpr Binding* binding{ nullptr };
  constexpr Body* body{ nullptr };
  run_phase<l>(binding, ctx, s);
  return run_phase<l>(body, ctx, s);
}

template <typename l, typename TranCtx, typename store, typename Binding, typename Body>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template LetRemote<Binding, Body>>*, TranCtx& ctx, store& s)
{
  constexpr Binding* binding{ nullptr };
  constexpr Body* body{ nullptr };
  run_phase<l>(binding, ctx, s);
  return run_phase<l>(body, ctx, s);
}

	template <typename l, typename TranCtx, typename store, typename name, typename expr, typename Body>
	auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template LetIsValid<name, expr, Body>>*, TranCtx& ctx, store& s)
{
  constexpr Body* body{ nullptr };
	assert(false && "need to run this binding");
  return run_phase<l>(body, ctx, s);
}
	
	template <typename l, typename TranCtx, typename store, typename name, typename expr, typename... args>
	auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template StatementOperation<name, expr, args...>>*, TranCtx& ctx, store& s)
{
	//returns void; this is the *non-expression* variant.
	constexpr OperationIdentifier<name> opname;
	constexpr expr* _expr{nullptr};
	auto hndl = run_phase<l>(_expr,ctx,s);
	auto& upCast = hndl.upCast(opname);
	auto &op = upCast.op;
	assert(op);
	auto &dop = *op;
	dop.act(&ctx,hndl,run_phase<l>((args*)nullptr,ctx,s)...);
}

template <typename l, typename TranCtx, typename store, typename L, typename y, typename R>
auto _run_phase(typename AST<l>::template Statement<
                  typename AST<l>::template Assignment<typename AST<l>::template Expression<y, typename AST<l>::template VarReference<L>>, R>>*,
                TranCtx& ctx, store& s)
{
  constexpr R* r{ nullptr };
  s.get(L{}).push(s,ctx, run_phase<l>(r, ctx, s));
}

template <typename l, typename TranCtx, typename store, typename y, typename R>
y
_run_phase(typename AST<l>::template Statement<typename AST<l>::template Return<typename AST<l>::template Expression<y, R>>>*, TranCtx& ctx, store& s,
           std::enable_if_t<!std::is_void<y>::value>* = nullptr)
{
  constexpr typename AST<l>::template Expression<y, R>* r{ nullptr };
  whendebug(auto &s2 = s.as_virtual_holder().template get_specific_holder<int>());
  whendebug((void)s2);
  throw ReturnedValue<y>{ run_phase<l>(r, ctx, s) };
}

template <typename l, typename TranCtx, typename store, typename y, typename R>
void
_run_phase(typename AST<l>::template Statement<typename AST<l>::template AccompanyWrite<typename AST<l>::template Expression<y, typename AST<l>::template VarReference<R>>>>*, TranCtx& ctx, store& s)
{
#ifdef TRACK
  ctx.trk_ctx.trk.accompanyWrite(ctx,s.get(R{}).get_remote(ctx).name(),
								 s.get(typecheck_phase::tombstone_str{}).get(s,ctx).name());
#else
	(void) ctx;
	(void) s;
#endif
}

template <typename l, typename TranCtx, typename store, typename V>
void
_run_phase(typename AST<l>::template Statement<typename AST<l>::template WriteTombstone<typename AST<l>::template Expression<tracker::Tombstone, typename AST<l>::template VarReference<V>>>>*, TranCtx& ctx, store& s)
{
#ifdef TRACK
	ctx.trk_ctx.trk.writeTombstone(ctx, s.get(V{}).get(s,ctx).name());
#else
	(void) ctx;
	(void) s;
#endif
}

template <typename l, typename TranCtx, typename store>
tracker::Tombstone _run_phase(typename AST<l>::template Expression<tracker::Tombstone, typename AST<l>::template GenerateTombstone<>>*, TranCtx& , store&)
{
#ifdef TRACK
  return tracker::Tracker::generateTombstone();
#else
	(void) ctx;
	return tracker::Tombstone{0,0,0};
#endif
}

template <typename l, typename TranCtx, typename store, typename y1, typename y2, typename S, typename F, typename R>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template Assignment<
                  typename AST<l>::template Expression<
                    y1, typename AST<l>::template FieldReference<typename AST<l>::template Expression<y2, typename AST<l>::template VarReference<F>>, S>>,
                  R>>*,
                TranCtx& ctx, store& s)
{
	auto strct = s.get(F{}).get(s,ctx);
  constexpr R* rval{ nullptr };
  strct.field(S{}) = run_phase<l>(rval, ctx, s);
  s.get(F{}).push(s,ctx, strct);
}

template <typename l, typename TranCtx, typename store, char... var>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template IncrementOccurance<String<var...>>>*, TranCtx&, store& s)
{
  s.get(String<var...>{}).increment(s);
}

template <typename l, typename TranCtx, typename store, char... var>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template IncrementRemoteOccurance<String<var...>>>*, TranCtx&, store& s)
{
  s.get(String<var...>{}).increment_remote(s);
}

template <typename l, typename TranCtx, typename store, typename hndl_t, typename var>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template IncrementOccurance<typename AST<l>::template Expression<hndl_t,typename AST<l>::template VarReference<var> > > >*,
				TranCtx& ctx, store& s, std::enable_if_t<std::is_base_of<remote_map_holder<hndl_t>, store>::value >* = nullptr)
{
	//this is the version that runs on explicit handles
	//this superclass represents every remote handle of this type we have.
	remote_map_holder<hndl_t> &super = s;
	super.increment_matching(run_phase<l>(typename AST<l>::template Expression<hndl_t,typename AST<l>::template VarReference<var> >{},ctx,s));
}

template <typename l, typename TranCtx, typename store, typename hndl_t, typename var>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template IncrementOccurance<typename AST<l>::template Expression<hndl_t,typename AST<l>::template VarReference<var> > > >*,
				TranCtx& , store& , std::enable_if_t<!std::is_base_of<remote_map_holder<hndl_t>, store>::value >* = nullptr)
{
	//noop; no alias possible.
}

//TODO: we might want to find a way to only emit RefreshRemoteOccurance when there is the possibility of alias.
template <typename l, typename TranCtx, typename store, typename hndl_t, typename var>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template RefreshRemoteOccurance<typename AST<l>::template Expression<hndl_t,typename AST<l>::template VarReference<var> > > > *,
				TranCtx& ctx, store& s, std::enable_if_t<std::is_base_of<remote_map_holder<hndl_t>, store>::value >* = nullptr)
{
	remote_map_holder<hndl_t> &super = s;
	auto hndl = run_phase<l>(typename AST<l>::template Expression<hndl_t,typename AST<l>::template VarReference<var> >{},ctx,s);
	auto &_this_super = super.super[hndl.name()];
	_this_super.push(_this_super,ctx,*hndl.get(&ctx));
}

template <typename l, typename TranCtx, typename store, typename hndl_t, typename var>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template RefreshRemoteOccurance<typename AST<l>::template Expression<hndl_t,typename AST<l>::template VarReference<var> > > > *,
				TranCtx& , store& , std::enable_if_t<!std::is_base_of<remote_map_holder<hndl_t>, store>::value >* = nullptr)
{
	//noop; no alias possible.
}

template <typename l, typename TranCtx, typename store, typename c, typename t, typename e>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template If<c, t, e>>*, TranCtx& ctx, store& s)
{
  constexpr c* condition{ nullptr };
  constexpr t* then{ nullptr };
  constexpr e* els{ nullptr };
  if (run_phase<l>(condition, ctx, s))
    run_phase<l>(then, ctx, s);
  else
    run_phase<l>(els, ctx, s);
}

template <typename l, typename TranCtx, typename store, typename c, typename t, char... name>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template While<c, t, name...>>*, TranCtx& ctx, store& s)
{
  constexpr c* condition{ nullptr };
  constexpr t* then{ nullptr };
  int i = 0;
  s.get(String<name...>{}).bind(s,ctx, i);
  bool condition_val = run_phase<l>(condition, ctx, s);
  while (condition_val) {
    ++i;
    s.get(String<name...>{}).push(s,ctx, i);
    run_phase<l>(then, ctx, s);
    condition_val = run_phase<l>(condition, ctx, s);
  }
}

template <typename l, typename TranCtx, typename store, typename t, char... name>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template ForEach<t, name...>>*, TranCtx& ctx, store& s)
{
  constexpr t* then{ nullptr };
  constexpr String<name...> bound_name{};
  auto bound = s.get(bound_name).get(s,ctx);
  for (auto i = 0u; i < bound; ++i) {
    run_phase<l>(then, ctx, s);
  }
}

template <typename l, typename TranCtx, typename store>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template Sequence<>>*, TranCtx&, store&)
{
}

template <typename l, typename TranCtx, typename store, typename S1>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template Sequence<S1>>*, TranCtx& ctx, store& s)
{
  return run_phase<l>(((S1*)nullptr), ctx, s);
}

template <typename l, typename TranCtx, typename store, typename S1, typename S2, typename... Seq>
auto _run_phase(typename AST<l>::template Statement<typename AST<l>::template Sequence<S1, S2, Seq...>>*, TranCtx& ctx, store& s)
{
  constexpr S1* first{ nullptr };
  run_phase<l>(first, ctx, s);
  constexpr typename AST<l>::template Statement<typename AST<l>::template Sequence<S2, Seq...>>* next{ nullptr };
  return run_phase<l>(next, ctx, s);
}

template <typename l, typename AST, typename TranCtx, typename store>
auto run_phase(AST* a, TranCtx& ctx, store& s)
{
  return _run_phase<l>(a, ctx, s);
}

template <typename phase, typename TranCtx, typename store>
auto run_phase(TranCtx& ctx, store& s)
{
  constexpr typename phase::ast* np{ nullptr };
  return run_phase<typename phase::label>(np, ctx, s);
}

template <typename Ctx, typename S>
void
commit_phase(Ctx& ctx, S& old, S& s)
{
  if (ctx.s_ctx)
    ctx.s_ctx->store_commit();
  old.take(std::move(s));
}

template <typename phase1, typename Ctx, typename S>
void
run_phase1(Ctx& ctx, S& old, S& s, std::enable_if_t<std::is_void<typename phase1::returns>::value>* = nullptr)
{
  try {
    run_phase<phase1>(ctx, s);
  } catch (const ReturnedValue<void>&) {
  }
  commit_phase(ctx, old, s);
}

template <typename phase1, typename Ctx, typename S>
typename phase1::returns
run_phase1(Ctx& ctx, S& old, S& s, std::enable_if_t<!std::is_void<typename phase1::returns>::value>* = nullptr)
{
  try {
    run_phase<phase1>(ctx, s);
    throw mutils::StaticMyriaException<'n', 'o', 'n', '-', 'v', 'o', 'i', 'd', ' ', 'r', 'e', 't', 'u', 'r', 'n', 'i', 'n', 'g', ' ', 't', 'r', 'a', 'n', 's',
                                       'a', 'c', 't', 'i', 'o', 'n', ' ', 'f', 'a', 'i', 'l', 'e', 'd', ' ', 't', 'o', ' ', 'r', 'e', 't', 'u', 'r', 'n', ' ',
                                       'a', 'n', 'y', 't', 'h', 'i', 'n', 'g'>{};
  } catch (ReturnedValue<typename phase1::returns>& r) {
    commit_phase(ctx, old, s);
    return std::move(r.value);
  }
}

template <typename phase1, typename store>
auto common_interp_loop(store& old, store s, tracker::Tracker& trk)
{
  using label = typename phase1::label;
  PhaseContext<label> ctx{ trk };
  s.begin_phase();
  return run_phase1<phase1>(ctx, old, s);
}

template <typename phase1, typename store>
auto common_interp(store& s, tracker::Tracker& trk)
{
  using label = typename phase1::label;
  try {
    return common_interp_loop<phase1>(s, s.clone(), trk);
  } catch (const SerializationFailure& sf) {
    std::size_t backoff_multiplier{ 2 };
    while (!label::can_abort::value) {
      try {
				using namespace std; using namespace chrono;
				constexpr seconds max_backoff{5min};
        // try an exponential back-off strategy
				auto backoff = microseconds{ (size_t)mutils::better_rand() * backoff_multiplier + 1};
        std::this_thread::sleep_for(max_backoff > backoff ? backoff : max_backoff);
        backoff_multiplier *= 2;
        return common_interp_loop<phase1>(s, s.clone(), trk);
      } catch (const SerializationFailure& sf) {
				
      }
    } /*else if can_abort,*/
    throw sf;
  }
}
}
}
}
