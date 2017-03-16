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

template <typename store, typename TransactionContext, typename phase1>
auto interp2(transaction<phase1>*, TransactionContext& ctx, store& s)
{
  s.reset_indices();
  return run_phase<phase1>(ctx, s);
}

template <typename store, typename TransactionContext, typename phase1, typename phase2, typename... phase>
auto interp2(transaction<phase1, phase2, phase...>*, TransactionContext& ctx, store& s)
{
  s.reset_indices();
  run_phase<phase1>(ctx, s);
  constexpr transaction<phase2, phase...>* remains{ nullptr };
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
  store.reset_indices();
  return interp2(np, ctx, store);
}
}
}
}
