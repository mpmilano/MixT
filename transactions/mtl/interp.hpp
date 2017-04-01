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
		if (ctx.s_ctx) ctx.s_ctx->store_commit();
	}

	template<typename phase1, typename Ctx, typename S>
	auto run_phase1(Ctx &ctx, S &s, std::enable_if_t<!std::is_void<DECT(run_phase<phase1>(ctx, s))>::value >* = nullptr){
		auto ret = run_phase<phase1>(ctx, s);
		if (ctx.s_ctx) ctx.s_ctx->store_commit();
		return ret;
	}
	
	template<typename phase1, typename store>
	auto common_interp(store& s){
		using label = typename phase1::label;
		PhaseContext<label> ctx{tctx};
		do {
			try {
				ctx.reset();
				s.begin_phase();
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
auto interp2(transaction<phase1>*, store& s)
{
	return common_interp<phase1>(s);
}

template <typename store, typename phase1, typename phase2, typename... phase>
auto interp2(transaction<phase1, phase2, phase...>*, store& s)
{
	constexpr transaction<phase2, phase...>* remains{ nullptr };
	common_interp<phase1>(s);
	return interp2(remains, s);
}
	template <typename, typename split, typename store>
	auto interp3(void*, split* np, store& s){
		interp2(np, s);
	}

	template <typename ptr, typename split, typename store>
	auto interp3(ptr*, split* np, store& s, std::enable_if_t<!std::is_void<ptr>::value >* = nullptr){
		auto ret = interp2(np, s);
		return ret;
	}

template <typename split, typename... required>
auto begin_interp(required... vals)
{
  constexpr split* np{ nullptr };
  using store_t = typename split::template all_store<required...>;
  // required should be struct value<>
  static_assert(is_store<store_t>::value);
  store_t store{ vals... };
	using ret_t = DECT(interp2(np, store));
	constexpr ret_t *rt{nullptr};
	return interp3<ret_t>(rt, np, store);
}
}
}
}
