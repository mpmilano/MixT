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

	template<typename phase1, typename store>
	auto common_interp(TrackingContext& ctx, store& s){
		using label = typename phase1::label;
		do {
			try {
				s.begin_phase();
				struct PhaseContext{
					std::unique_ptr<StoreContext<label> > s_ctx;
					SingleTransactionContext<label> &sctx; //PhaseContext(DECT(sctx) &sctx):sctx(sctx){}
					~PhaseContext(){ 
						if (sctx.s_ctx && !sctx.s_ctx->aborted() && !sctx.s_ctx->committed()) sctx.s_ctx->store_commit();
						sctx.s_ctx.reset();
					} };
				PhaseContext ose{ctx};
				return run_phase<phase1>(ctx, s);
			}
			catch(const SerializationFailure& sf){
				s.rollback_phase();
				whendebug(if (!label::can_abort::value) std::cout << "Error: label which cannot abort aborted! " << label{});
				if (label::can_abort::value) throw sf;
		}
		}
		while(!label::can_abort::value);
	}
	
template <typename store, typename TransactionContext, typename phase1>
auto interp2(transaction<phase1>*, TransactionContext& ctx, store& s)
{
	return common_interp<phase1>(ctx,s);
}

template <typename store, typename TransactionContext, typename phase1, typename phase2, typename... phase>
auto interp2(transaction<phase1, phase2, phase...>*, TransactionContext& ctx, store& s)
{
	constexpr transaction<phase2, phase...>* remains{ nullptr };
	common_interp<phase1>(ctx,s);
	return interp2(remains, ctx, s);
}

template <typename split, typename TransactionContext, typename... required>
auto begin_interp(TransactionContext& ctx, required... vals)
{
  constexpr split* np{ nullptr };
  using store_t = typename split::template all_store<required...>;
  // required should be struct value<>
  static_assert(is_store<store_t>::value);
  store_t store{ vals... };
	auto ret = interp2(np, ctx, store);
	ctx.commitContext();
	return ret;
}
}
}
}
