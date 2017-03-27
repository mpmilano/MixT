#pragma once
#include "parse_bindings.hpp"
#include "parse_expressions.hpp"
#include "parse_statements.hpp"
#include "flatten_expressions.hpp"
#include "label_inference.hpp"
#include "split_phase.hpp"
#include "interp.hpp"
#include "CTString.hpp"
#include "recollapse.hpp"
#include <memory>
#include "Basics.hpp"


namespace myria {
namespace mtl {

template <typename>
struct pre_transaction_str;

template <typename split, typename... bound_values>
struct transaction_struct
{
  constexpr transaction_struct() = default;
  using transaction = split;

  static auto run(tracker::Tracker& trk, const typename bound_values::type&... v)
  {
    using namespace runnable_transaction;
		tracker::TrackingContext ctx{trk};
    return begin_interp<transaction>(ctx, bound_values{ v }...);
  }
};

template <char... Str>
struct pre_transaction_str<mutils::String<Str...>>
{
  using transaction_text = mutils::String<Str...>;
	template<typename label> using requires_tracking = typename label::requires_causal_tracking;

  template <typename... bound_values>
  constexpr static auto compile()
  {
    using parsed_t = DECT(parse_statement(transaction_text{}));
    {
      using namespace parse_phase;
      using flattened_t = DECT(parse_phase::flatten_expressions(parsed_t{}));
      {
        using namespace typecheck_phase;
        using checked_t = DECT(typecheck_phase::typecheck<1, 1>(typecheck_phase::type_environment<Label<top>, bound_values...>{}, flattened_t{}));
        {
          using namespace label_inference;
          using inferred_t = DECT(infer_labels(checked_t{}));
          {
            using namespace split_phase;
            using split_t = DECT(split_computation<inferred_t, bound_values...>());
						//this is where we should introduce the tombstones, I think. if (labels::exists_predicate<requires_tracking>()){}
            using recollapsed_t = DECT(recollapse(split_t{}));
            return recollapsed_t{};
          }
        }
      }
    }
  }

  template <typename... value>
  static constexpr auto with()
  {
    constexpr auto split = compile<type_binding<typename value::name, typename value::type, Label<top>, type_location::local>...>();
    return transaction_struct<DECT(split), value...>{};
  }
};
}
}
