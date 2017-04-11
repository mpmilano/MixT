#pragma once

#include "top.hpp"
#include "AST_split.hpp"

namespace myria {
namespace pgsql {

struct causal {};
struct strong {};

enum class Level { causal, strong, MAX };
std::ostream &operator<<(std::ostream &o, const Level &);
}
template <> struct Label<pgsql::causal> {

  constexpr Label() = default;

  constexpr static bool flows_to(const Label<bottom> &) { return true; }

  constexpr static bool flows_to(const Label<top> &) { return false; }

  constexpr static bool flows_to(const Label<pgsql::strong> &) { return false; }

  constexpr static bool flows_to(const Label &) { return true; }

  template <typename... lbls> constexpr static auto min_with(const lbls &...) {
    return std::conditional_t<
        mutils::exists<std::is_same<lbls, Label<bottom>>::value...>(),
        Label<bottom>, Label>{};
  }

  template <typename... lbls> constexpr static bool is_min_of(const lbls &...) {
    constexpr bool res2 = (lbls::flows_to(Label{}) && ... && true);
    constexpr bool res =
        !mutils::exists<std::is_same<lbls, Label<bottom>>::value...>();
    static_assert(res == res2);
    return res;
  }

  template <typename... lbls> constexpr static bool is_max_of(const lbls &...) {
    constexpr bool res2 = (Label::flows_to(lbls{}) && ... && true);
    constexpr bool res =
        !(mutils::exists<std::is_same<lbls, Label<top>>::value...>() ||
          mutils::exists<std::is_same<lbls, Label<pgsql::strong>>::value...>());
    static_assert(res == res2);
    return res;
  }

  using is_strong = std::false_type;
  using is_causal = std::true_type;

  using might_track = std::true_type;
  template <typename T>
  using tracks_against = std::is_same<T, Label<pgsql::strong>>;
  using can_abort = std::false_type;
  using is_label = std::true_type;
  using run_remotely = std::true_type;
  using int_id = std::integral_constant<std::size_t, 25>;

  static constexpr char description[] = "causal";
};

constexpr auto parse_label(mutils::String<'c', 'a', 'u', 's', 'a', 'l'>) {
  return Label<pgsql::causal>{};
}

template <> struct Label<pgsql::strong> {

  constexpr Label() = default;

  constexpr static bool flows_to(const Label<bottom> &) { return true; }

  constexpr static bool flows_to(const Label<top> &) { return false; }

  constexpr static bool flows_to(const Label<pgsql::causal> &) { return true; }

  constexpr static bool flows_to(const Label &) { return true; }

  template <typename... lbls> constexpr static auto min_with(const lbls &...) {
    return std::conditional_t<
        mutils::exists<std::is_same<lbls, Label<bottom>>::value...>(),
        Label<bottom>,
        std::conditional_t<mutils::exists<std::is_same<
                               lbls, Label<pgsql::causal>>::value...>(),
                           Label<pgsql::causal>, Label>>{};
  }

  template <typename... lbls> constexpr static bool is_min_of(const lbls &...) {
    constexpr bool res2 = (lbls::flows_to(Label{}) && ... && true);
    constexpr bool res =
        !(mutils::exists<std::is_same<lbls, Label<bottom>>::value...>() ||
          mutils::exists<std::is_same<lbls, Label<pgsql::causal>>::value...>());
    static_assert(res == res2);
    return res;
  }

  template <typename... lbls> constexpr static bool is_max_of(const lbls &...) {
    constexpr bool res2 = (Label::flows_to(lbls{}) && ... && true);
    constexpr bool res =
        !mutils::exists<std::is_same<lbls, Label<top>>::value...>();
    static_assert(res == res2);
    return res;
  }

  using is_strong = std::true_type;
  using is_causal = std::false_type;

  using might_track = std::false_type;
  template <typename> using tracks_against = std::false_type;
  using can_abort = std::true_type;
  using is_label = std::true_type;
  using run_remotely = std::true_type;
  using int_id = std::integral_constant<std::size_t, 50>;

  static constexpr char description[] = "strong";
};
constexpr auto parse_label(mutils::String<'s', 't', 'r', 'o', 'n', 'g'>) {
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
