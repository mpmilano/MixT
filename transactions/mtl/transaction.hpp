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

namespace myria {
namespace mtl {

	template <txnID_t, typename>
struct pre_transaction_str;
	
	template <std::size_t num_remote, typename split, typename... bound_values>
	struct transaction_struct;

	template <typename split, typename... bound_values>
	struct transaction_struct<1,split,bound_values...>
	{
		constexpr transaction_struct() = default;
		using transaction = split;
		template<typename label> using find_phase = typename transaction::template find_phase<label>;

	private:
		template<typename run_remotely>
		static auto interp(mutils::DeserializationManager* dsm, mutils::connection *c, const typename bound_values::type&... v){
			using namespace runnable_transaction;
			using namespace mutils;
			return begin_interp<transaction,mutils::array<connection*,1,connection*>,run_remotely,bound_values...>
				(dsm,mutils::array<connection*,1,connection*>{c},bound_values{ v }...);	
		}
	public:
		
		static auto run_optimistic(mutils::DeserializationManager* dsm, mutils::connection &c, const typename bound_values::type&... v){
			return interp<std::true_type>(dsm,&c,v...);
		}
		static auto run_local(const typename bound_values::type&... v){
			return interp<std::false_type>(nullptr,nullptr,v...);
		}

		using all_store = typename transaction::template all_store<bound_values...>;
	};
	template <std::size_t num_remote, typename split, typename... bound_values>
	std::ostream& operator<<(std::ostream& o, transaction_struct<num_remote,split,bound_values...>){
		return o << split{};
	}

	template <txnID_t id, char... Str>
	struct pre_transaction_str<id,mutils::String<Str...>>
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
            using split_t = DECT(split_computation<id, inferred_t, bound_values...>());
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
    return transaction_struct<DECT(split)::number_remote_phases::value, DECT(split), value...>{};
  }
};
}
}
