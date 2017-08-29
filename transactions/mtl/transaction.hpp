#pragma once
#include "parse_bindings.hpp"
#include "parse_expressions.hpp"
#include "better_constructable_array.hpp"
#include "parse_statements.hpp"
#include "flatten_expressions.hpp"
#include "label_inference.hpp"
#include "split_phase.hpp"
#include "interp.hpp"
#include "CTString.hpp"
#include "recollapse.hpp"
#include <memory>
#include "Basics.hpp"
#include "transaction_listener.hpp"
#include "mtlbasics.hpp"
#include "insert_tracking.hpp"
#include "endorse_relabel.hpp"
//*/
namespace myria {
namespace mtl {

template <typename>
struct pre_transaction_str;

template <std::size_t num_remote, typename previous_transaction_phases, typename split, typename... bound_values>
struct transaction_struct;

template <typename _inferred, typename... value>
struct previous_transaction_phases
{
  using inferred = _inferred;

  template <typename tracked>
  struct resume_compilation_inferred_str
  {
    using recollapsed =
      DECT(recollapse(split_phase::split_computation<tracked, type_binding<typename value::name, typename value::type, Label<top>, type_location::local>...>()));
		using all_store = typename recollapsed::template all_store<value...>;
  };

  template <typename tracked>
  using resume_compilation_inferred = typename resume_compilation_inferred_str<tracked>::recollapsed;
};

  template <std::size_t n,  typename _previous_transaction_phases, typename split, typename... bound_values>
  struct transaction_struct
  {
    constexpr transaction_struct() = default;
    using transaction = split;
    using previous_transaction_phases = _previous_transaction_phases;
    template <typename label>
    using find_phase = typename transaction::template find_phase<label>;
  private:
    template <typename run_remotely, typename ClientTracker, typename connections>
    static auto interp(ClientTracker& trk, mutils::DeserializationManager* dsm, const connections &c, const typename bound_values::type&... v)
    {
      using namespace runnable_transaction;
      using namespace mutils;
      return begin_interp<previous_transaction_phases, transaction, connections, run_remotely, ClientTracker, bound_values...>(
        dsm, trk, c, bound_values{ v }...);
    }
  public:
    template <typename ClientTracker>
    static auto run_optimistic(ClientTracker& trk, mutils::DeserializationManager* dsm, const typename ClientTracker::connection_references& c,
                               const typename bound_values::type&... v)
    {
      return transaction_struct::template interp<std::true_type>(trk, dsm, c, v...);
    }
    template <typename ClientTracker>
    static auto run_local(ClientTracker& trk, const typename bound_values::type&... v)
    {
      return transaction_struct::template interp<std::false_type>(trk, nullptr, mutils::mismatch{}, v...);
    }
    using all_store = typename transaction::template all_store<bound_values...>;
  };

template <typename _previous_transaction_phases, typename split, typename... bound_values>
struct transaction_struct<0, _previous_transaction_phases, split, bound_values...>
{
  constexpr transaction_struct() = default;
  using transaction = split;
  using previous_transaction_phases = _previous_transaction_phases;
  template <typename label>
  using find_phase = typename transaction::template find_phase<label>;
  template <typename ClientTracker>
  static auto run_local(ClientTracker& trk, const typename bound_values::type&... v)
  {
    using namespace runnable_transaction;
    using namespace mutils;
    return begin_interp<previous_transaction_phases, transaction, mutils::mismatch, std::false_type, ClientTracker, bound_values...>(
	nullptr, trk, mutils::mismatch{}, bound_values{ v }...);
  }
  using all_store = typename transaction::template all_store<bound_values...>;
};

  template <std::size_t num_remote, typename previous_phases, typename split, typename... bound_values>
  std::ostream& operator<<(std::ostream& o, transaction_struct<num_remote, previous_phases, split, bound_values...>)
{
  return o << split{};
}

template <char... Str>
struct pre_transaction_str<mutils::String<Str...>>
{
  using transaction_text = mutils::String<Str...>;
  template <typename label>
  using requires_tracking = typename label::requires_causal_tracking;

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
            using namespace tracking_phase;
            using tracked_t = DECT(insert_tracking_begin(inferred_t{}));
			using endorsed_one_t = DECT(do_pre_endorse(tracked_t{}));
			static_assert(!Contains_endorsement<contains_endorsement_argument<false,mutils::EmptyWorkList>,  endorsed_one_t>::value);
            using namespace split_phase;
            using split_t = DECT(split_computation<endorsed_one_t, bound_values...>());
            using recollapsed_t = DECT(recollapse(split_t{}));
            struct inferred_and_recollapsed
            {
              constexpr inferred_and_recollapsed() = default;
              using inferred = DECT(do_pre_endorse(inferred_t{}));
              using recollapsed = recollapsed_t;
            };
            return inferred_and_recollapsed{};
          }
        }
      }
    }
  }

  template <typename... value>
  static constexpr auto with()
  {
    constexpr auto inferred_and_recollapsed = compile<type_binding<typename value::name, typename value::type, Label<top>, type_location::local>...>();
    using recollapsed = typename DECT(inferred_and_recollapsed)::recollapsed;
    using inferred = typename DECT(inferred_and_recollapsed)::inferred;
    using previous_phases = previous_transaction_phases<inferred, value...>;
    return transaction_struct<recollapsed::number_remote_phases::value, previous_phases, recollapsed, value...>{};
  }
};
}
}
