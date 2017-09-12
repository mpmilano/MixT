#pragma once

#include "../mtl/top.hpp"
#include "../mtl/AST_split.hpp"
#include "../mtl/pre_endorse.hpp"

namespace myria {
namespace pgsql {

	using causal = mutils::String<'c', 'a', 'u', 's', 'a', 'l'>;
	using strong = mutils::String<'s', 't', 'r', 'o', 'n', 'g'>;

enum class Level { causal, strong, MAX };
std::ostream &operator<<(std::ostream &o, const Level &);
}
template <> struct Label<pgsql::causal> {

  constexpr Label() = default;

  constexpr static bool flows_to(const Label<bottom> &) { return true; }

  constexpr static bool flows_to(const Label<top> &) { return false; }

	template<typename l>
	constexpr static bool flows_to(const Label<PreEndorse<l>> &) { return false; }

  constexpr static bool flows_to(const Label<pgsql::strong> &) { return false; }

  constexpr static bool flows_to(const Label &) { return true; }

  using is_strong = std::false_type;
  using is_causal = std::true_type;

  using might_track = std::true_type;
  template <typename T>
  using tracks_against = std::is_same<T, Label<pgsql::strong>>;
  using can_abort = std::false_type;
  using is_label = std::true_type;
  using run_remotely = std::true_type;
  using int_id = std::integral_constant<std::size_t, 25>;

  using description = pgsql::causal;
};

constexpr auto parse_label(pgsql::causal) {
  return Label<pgsql::causal>{};
}

template <> struct Label<pgsql::strong> {

  constexpr Label() = default;

  constexpr static bool flows_to(const Label<bottom> &) { return true; }

  constexpr static bool flows_to(const Label<top> &) { return false; }

	template<typename l>
	constexpr static bool flows_to(const Label<PreEndorse<l>> &) { return false; }

  constexpr static bool flows_to(const Label<pgsql::causal> &) { return true; }

  constexpr static bool flows_to(const Label &) { return true; }

  using is_strong = std::true_type;
  using is_causal = std::false_type;

  using might_track = std::false_type;
  template <typename> using tracks_against = std::false_type;
  using can_abort = std::true_type;
  using is_label = std::true_type;
  using run_remotely = std::true_type;
  using int_id = std::integral_constant<std::size_t, 50>;

  using description = pgsql::strong;
};
constexpr auto parse_label(pgsql::strong) {
  return Label<pgsql::strong>{};
}

std::ostream &operator<<(std::ostream &o, const Label<pgsql::strong> &);
std::ostream &operator<<(std::ostream &o, const Label<pgsql::causal> &);

namespace mtl {
namespace split_phase {
static_assert(!are_equivalent(Label<top>{}, Label<pgsql::strong>{}));
static_assert(!are_equivalent(Label<top>{}, Label<pgsql::causal>{}));
static_assert(!are_equivalent(Label<top>{}, Label<bottom>{}));
static_assert(!are_equivalent(Label<bottom>{}, Label<pgsql::strong>{}));
static_assert(!are_equivalent(Label<bottom>{}, Label<pgsql::causal>{}));
static_assert(!are_equivalent(Label<pgsql::causal>{}, Label<pgsql::strong>{}));
}
}
}
