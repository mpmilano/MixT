#pragma once
#include "mtl/runnable_transaction.hpp"
#include "mtl/phase_context.hpp"
#include "mtl/AST_typecheck.hpp"
#include "mtl/split_context.hpp"
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

	BEGIN_SPLIT_CONTEXT(runtime);
	
template <typename AST, typename TranCtx, typename store>
static auto run_phase(AST*, TranCtx&, store& s);

template <typename TranCtx, typename store, typename y, typename str, typename f>
static auto _run_phase(Expression<y, FieldReference<str, f>>*, TranCtx& ctx, store& s)
{
  constexpr str* next{ nullptr };
  return run_phase(next, ctx, s).field(f{});
}

template <typename TranCtx, typename store, typename y, typename v>
static auto _run_phase(Expression<y, VarReference<v>>*, TranCtx& ctx, store& s)
{
	constexpr auto explicit_v = v{};
	return s.get(explicit_v).get(s,ctx);
}

template <typename TranCtx, typename store, int i>
static auto _run_phase(Expression<int, Constant<i>>*, TranCtx&, store&)
{
  return i;
}

template <typename TranCtx, typename store, typename y, typename L, typename R>
static auto _run_phase(Expression<y, BinOp<'+', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase(nextl, ctx, s) + run_phase(nextR, ctx, s);
}

template <typename TranCtx, typename store, typename y, typename L, typename R>
static auto _run_phase(Expression<y, BinOp<'-', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase(nextl, ctx, s) - run_phase(nextR, ctx, s);
}

template <typename TranCtx, typename store, typename y, typename L, typename R>
static auto _run_phase(Expression<y, BinOp<'*', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase(nextl, ctx, s) * run_phase(nextR, ctx, s);
}

template <typename TranCtx, typename store, typename y, typename L, typename R>
static auto _run_phase(Expression<y, BinOp<'/', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase(nextl, ctx, s) / run_phase(nextR, ctx, s);
}

template <typename TranCtx, typename store, typename y, typename L, typename R>
static auto _run_phase(Expression<y, BinOp<'=', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase(nextl, ctx, s) == run_phase(nextR, ctx, s);
}

template <typename TranCtx, typename store, typename y, typename L, typename R>
static auto _run_phase(Expression<y, BinOp<'&', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase(nextl, ctx, s) && run_phase(nextR, ctx, s);
}

template <typename TranCtx, typename store, typename y, typename L, typename R>
static auto _run_phase(Expression<y, BinOp<'|', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase(nextl, ctx, s) || run_phase(nextR, ctx, s);
}

template <typename TranCtx, typename store, typename y, typename L, typename R>
static auto _run_phase(Expression<y, BinOp<'>', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  return run_phase(nextl, ctx, s) > run_phase(nextR, ctx, s);
}

template <typename TranCtx, typename store, typename y, typename L, typename R>
static auto _run_phase(Expression<y, BinOp<'<', L, R>>*, TranCtx& ctx, store& s)
{
  constexpr L* nextl{ nullptr };
  constexpr R* nextR{ nullptr };
  auto lval = run_phase(nextl, ctx, s);
  auto rval = run_phase(nextR, ctx, s);
  return lval < rval;
}

	template <typename TranCtx, typename store, typename y, typename expr>
	static auto _run_phase(Expression<y,IsValid<expr>>*, TranCtx& ctx, store& s)
{
	constexpr expr* hndl{ nullptr };
    auto ret = run_phase(hndl, ctx, s).isValid(&ctx);
    return ret;
}

template <typename TranCtx, typename store, typename label, typename Yield, typename Expr, char... name>
static auto _run_phase(Binding<Label<label>, Yield, String<name...>, Expr>*, TranCtx& ctx, store& s)
{
  constexpr Expr* expr{ nullptr };
  constexpr String<name...> varname{};
  (void)varname;
  auto expr_result = run_phase(expr, ctx, s);
  s.get(String<name...>{}).bind(s,ctx, expr_result);
}

template <typename TranCtx, typename store, typename Binding, typename Body>
static auto _run_phase(Statement<Let<Binding, Body>>*, TranCtx& ctx, store& s)
{
  constexpr Binding* binding{ nullptr };
  constexpr Body* body{ nullptr };
  run_phase(binding, ctx, s);
  return run_phase(body, ctx, s);
}

template <typename TranCtx, typename store, typename Binding, typename Body>
static auto _run_phase(Statement<LetRemote<Binding, Body>>*, TranCtx& ctx, store& s)
{
  constexpr Binding* binding{ nullptr };
  constexpr Body* body{ nullptr };
  run_phase(binding, ctx, s);
  return run_phase(body, ctx, s);
}
	
	template <typename TranCtx, typename store, typename name, typename expr, typename... args>
	static auto _run_phase(Operation<name, expr, args...>*, TranCtx& ctx, store& s)
{
	constexpr OperationIdentifier<name> opname;
	constexpr expr* _expr{nullptr};
	auto hndl = run_phase(_expr,ctx,s);
	auto& upCast = hndl.upCast(opname);
	auto &op = upCast.op;
	assert(op);
	auto &dop = *op;
	return dop.act(&ctx,hndl,run_phase((args*)nullptr,ctx,s)...);
}

	template <typename TranCtx, typename store, typename name, typename expr, typename... args>
	static auto _run_phase(Statement<Operation<name, expr, args...>>*, TranCtx& ctx, store& s)
{
	//returns void; this is the *non-expression* variant.
	constexpr Operation<name, expr, args...>* recur{nullptr};
	_run_phase<l, TranCtx, store, name, expr, args...>(recur,ctx,s);
}

	template <typename TranCtx, typename store, typename y, typename name, typename expr, typename... args>
	static auto _run_phase(Expression<y,Operation<name, expr, args...>>*, TranCtx& ctx, store& s)
{
	constexpr Operation<name, expr, args...>* recur{nullptr};
	return _run_phase<l, TranCtx, store, name, expr, args...>(recur,ctx,s);
}

template <typename TranCtx, typename store, typename L, typename y, typename R>
static auto _run_phase(Statement<
											 Assignment<Expression<y, VarReference<L> >, R>>*,
                TranCtx& ctx, store& s)
{
  constexpr R* r{ nullptr };
  s.get(L{}).push(s,ctx, run_phase(r, ctx, s));
}

template <typename TranCtx, typename store, typename y, typename R>
static y _run_phase(Statement<Return<Expression<y, R>>>*, TranCtx& ctx, store& s,
           std::enable_if_t<!std::is_void<y>::value>* = nullptr)
{
  constexpr Expression<y, R>* r{ nullptr };
  throw ReturnedValue<y>{ run_phase(r, ctx, s) };
}

template <typename TranCtx, typename store, typename y, typename R>
static void _run_phase(Statement<AccompanyWrite<Expression<y, VarReference<R>>>>*, TranCtx& ctx, store& s)
{
#ifdef TRACK
  ctx.trk_ctx.trk.accompanyWrite(ctx,s.get(R{}).get_remote(ctx).name(),
								 s.get(typecheck_phase::tombstone_str{}).get(s,ctx).name());
#else
	(void) ctx;
	(void) s;
#endif
}

template <typename TranCtx, typename store, typename V>
static void _run_phase(Statement<WriteTombstone<Expression<tracker::Tombstone, VarReference<V>>>>*, TranCtx& ctx, store& s)
{
#ifdef TRACK
	ctx.trk_ctx.trk.writeTombstone(ctx, s.get(V{}).get(s,ctx).name());
#else
	(void) ctx;
	(void) s;
#endif
}

template <typename TranCtx, typename store>
static tracker::Tombstone _run_phase(Expression<tracker::Tombstone, GenerateTombstone>*, TranCtx& , store&)
{
#ifdef TRACK
  return tracker::Tracker::generateTombstone();
#else
	return tracker::Tombstone{0,0,0};
#endif
}

template <typename TranCtx, typename store, typename y1, typename y2, typename S, typename F, typename R>
static auto _run_phase(Statement<Assignment<
                  Expression<
                    y1, FieldReference<Expression<y2, VarReference<F>>, S>>,
                  R>>*,
                TranCtx& ctx, store& s)
{
	auto strct = s.get(F{}).get(s,ctx);
  constexpr R* rval{ nullptr };
  strct.field(S{}) = run_phase(rval, ctx, s);
  s.get(F{}).push(s,ctx, strct);
}

template <typename TranCtx, typename store, char... var>
static auto _run_phase(Statement<IncrementOccurance<String<var...>>>*, TranCtx&, store& s)
{
  s.get(String<var...>{}).increment(s);
}

template <typename TranCtx, typename store, char... var>
static auto _run_phase(Statement<IncrementRemoteOccurance<String<var...>>>*, TranCtx&, store& s)
{
  s.get(String<var...>{}).increment_remote(s);
}

template <typename TranCtx, typename store, typename hndl_t, typename var>
static auto _run_phase(Statement<IncrementOccurance<Expression<hndl_t,VarReference<var> > > >*,
				TranCtx& ctx, store& s, std::enable_if_t<std::is_base_of<remote_map_holder<hndl_t>, store>::value >* = nullptr)
{
	//this is the version that runs on explicit handles
	//this superclass represents every remote handle of this type we have.
	remote_map_holder<hndl_t> &super = s;
	super.increment_matching(run_phase(Expression<hndl_t,VarReference<var> >{},ctx,s));
}

template <typename TranCtx, typename store, typename hndl_t, typename var>
static auto _run_phase(Statement<IncrementOccurance<Expression<hndl_t,VarReference<var> > > >*,
				TranCtx& , store& , std::enable_if_t<!std::is_base_of<remote_map_holder<hndl_t>, store>::value >* = nullptr)
{
	//noop; no alias possible.
}

//TODO: we might want to find a way to only emit RefreshRemoteOccurance when there is the possibility of alias.
template <typename TranCtx, typename store, typename hndl_t, typename var>
static auto _run_phase(Statement<RefreshRemoteOccurance<Expression<hndl_t,VarReference<var> > > > *,
				TranCtx& ctx, store& s, std::enable_if_t<std::is_base_of<remote_map_holder<hndl_t>, store>::value >* = nullptr)
{
	remote_map_holder<hndl_t> &super = s;
	auto hndl = run_phase(Expression<hndl_t,VarReference<var> >{},ctx,s);
	auto &_this_super = super.super[hndl.name()];
	_this_super.push(_this_super,ctx,*hndl.get(&ctx));
}

template <typename TranCtx, typename store, typename hndl_t, typename var>
static auto _run_phase(Statement<RefreshRemoteOccurance<Expression<hndl_t,VarReference<var> > > > *,
				TranCtx& , store& , std::enable_if_t<!std::is_base_of<remote_map_holder<hndl_t>, store>::value >* = nullptr)
{
	//noop; no alias possible.
}

template <typename TranCtx, typename store, typename c, typename t, typename e>
static auto _run_phase(Statement<If<c, t, e>>*, TranCtx& ctx, store& s)
{
  constexpr c* condition{ nullptr };
  constexpr t* then{ nullptr };
  constexpr e* els{ nullptr };
  if (run_phase(condition, ctx, s))
    run_phase(then, ctx, s);
  else
    run_phase(els, ctx, s);
}

template <typename TranCtx, typename store, typename c, typename t, char... name>
static auto _run_phase(Statement<While<c, t, name...>>*, TranCtx& ctx, store& s)
{
  constexpr c* condition{ nullptr };
  constexpr t* then{ nullptr };
  int i = 0;
  s.get(String<name...>{}).bind(s,ctx, i);
  bool condition_val = run_phase(condition, ctx, s);
  while (condition_val) {
    ++i;
    s.get(String<name...>{}).push(s,ctx, i);
    run_phase(then, ctx, s);
    condition_val = run_phase(condition, ctx, s);
  }
}

template <typename TranCtx, typename store, typename t, char... name>
static auto _run_phase(Statement<ForEach<t, name...>>*, TranCtx& ctx, store& s)
{
  constexpr t* then{ nullptr };
  constexpr String<name...> bound_name{};
  auto bound = s.get(bound_name).get(s,ctx);
  for (auto i = 0u; i < bound; ++i) {
    run_phase(then, ctx, s);
  }
}

template <typename TranCtx, typename store>
static auto _run_phase(Statement<Sequence<>>*, TranCtx&, store&)
{
}

template <typename TranCtx, typename store, typename S1>
static auto _run_phase(Statement<Sequence<S1>>*, TranCtx& ctx, store& s)
{
  return run_phase(((S1*)nullptr), ctx, s);
}

template <typename TranCtx, typename store, typename S1, typename S2, typename... Seq>
static auto _run_phase(Statement<Sequence<S1, S2, Seq...>>*, TranCtx& ctx, store& s)
{
  constexpr S1* first{ nullptr };
  run_phase(first, ctx, s);
  constexpr Statement<Sequence<S2, Seq...>>* next{ nullptr };
  return run_phase(next, ctx, s);
}

	END_SPLIT_CONTEXT;
	template <typename l>
	template <typename AST, typename TranCtx, typename store>
	
	auto runtime<l>::run_phase(AST* a, TranCtx& ctx, store& s)
	{
		return _run_phase(a, ctx, s);
	}


template <typename phase, typename TranCtx, typename store>
auto run_phase(TranCtx& ctx, store& s)
{
  constexpr typename phase::ast* np{ nullptr };
  return runtime<typename phase::label>::run_phase(np, ctx, s);
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
