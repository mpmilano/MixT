#pragma once

#pragma once
#include "AST_split.hpp"
#include "split_printer.hpp"
#include "environments.hpp"
#include "runnable_transaction.hpp"
#include "run_phase.hpp"

namespace myria {
namespace mtl {
namespace runnable_transaction {

	template<typename phase1, typename Ctx, typename S>
	void run_phase1(Ctx &ctx, S &s, std::enable_if_t<std::is_void<DECT(run_phase<phase1>(ctx, s))>::value >* = nullptr){
		run_phase<phase1>(ctx, s);
		ctx.s_ctx->store_commit();
	}

	template<typename phase1, typename Ctx, typename S>
	auto run_phase1(Ctx &ctx, S &s, std::enable_if_t<!std::is_void<DECT(run_phase<phase1>(ctx, s))>::value >* = nullptr){
		auto ret = run_phase<phase1>(ctx, s);
		ctx.s_ctx->store_commit();
		return ret;
	}
	
	template<typename phase1, typename store>
	auto common_interp(tracker::TrackingContext& tctx, store& s){
		using label = typename phase1::label;
		do {
			try {
				s.begin_phase();
				PhaseContext<label> ctx{tctx};
				return run_phase1<phase1>(ctx,s);
			}
			catch(const SerializationFailure& sf){
				s.rollback_phase();
				whendebug(if (!label::can_abort::value) std::cout << "Error: label which cannot abort aborted! " << label{});
				if (label::can_abort::value) throw sf;
		}
		}
		while(!label::can_abort::value);
	}
	
template <typename store,typename phase1>
auto interp2(transaction<phase1>*, tracker::TrackingContext& ctx, store& s)
{
	return common_interp<phase1>(ctx,s);
}

template <typename store, typename phase1, typename phase2, typename... phase>
auto interp2(transaction<phase1, phase2, phase...>*, tracker::TrackingContext &ctx, store& s)
{
	constexpr transaction<phase2, phase...>* remains{ nullptr };
	common_interp<phase1>(ctx,s);
	return interp2(remains, ctx, s);
}
	template <typename, typename split, typename tctx, typename store>
	auto interp3(void*, split* np, tctx& ctx, store& s){
		interp2(np, ctx, s);
		ctx.commitContext();
	}

	template <typename ptr, typename split, typename tctx, typename store>
	auto interp3(ptr*, split* np, tctx& ctx, store& s, std::enable_if_t<!std::is_void<ptr>::value >* = nullptr){
		auto ret = interp2(np, ctx, s);
		ctx.commitContext();
		return ret;
	}

template <typename split, typename... required>
auto begin_interp(tracker::TrackingContext& ctx, required... vals)
{
  constexpr split* np{ nullptr };
  using store_t = typename split::template all_store<required...>;
  // required should be struct value<>
  static_assert(is_store<store_t>::value);
  store_t store{ vals... };
	using ret_t = DECT(interp2(np, ctx, store));
	constexpr ret_t *rt{nullptr};
	return interp3<ret_t>(rt, np, ctx, store);
}
}
}
}
