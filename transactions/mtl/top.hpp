#pragma once
#include "mtlutils.hpp"

namespace myria {
template <typename>
struct Label;
struct top;
template <>
struct Label<top>
{

  constexpr Label() = default;

  using is_label = std::true_type;

  template <typename T>
  constexpr static bool flows_to(const Label<T>&)
  {
    return true;
  }

  template <typename T1, typename... T2>
  constexpr static auto min_with(const Label<T1>&, const T2&... t2)
  {
    return Label<T1>::min_with(t2...);
  }

  constexpr static Label<top> min_with() { return Label<top>{}; }

  template <typename... labels>
  constexpr static bool is_min_of(const labels&...)
  {
    return (true && ... && labels::flows_to(Label{}));
  }

  template <typename... labels>
  constexpr static bool is_max_of(const labels&...)
  {
    return true;
  }

	using requires_causal_tracking = std::false_type;
	using can_abort = std::false_type;
};

template <int, int>
struct temp_label;
// temporary labels which have not been inferred yet.
template <int seq, int depth>
struct Label<temp_label<seq, depth>>
{
  // you better not be trying to call flow analysis stuff on this label.
};

template <typename>
struct is_temp_label;
template <int a, int b>
struct is_temp_label<Label<temp_label<a, b>>> : public std::true_type
{
};
template <int a, int b>
struct is_temp_label<temp_label<a, b>> : public std::true_type
{
};
template <typename>
struct is_temp_label : public std::false_type
{
};

template <typename l, typename r>
struct label_min_of;

template <int l1, int l2, typename r>
struct Label<label_min_of<Label<temp_label<l1, l2>>, Label<r>>>
{
  static constexpr auto resolve() { return Label{}; }
  static constexpr bool can_resolve() { return false; }
};

template <int l1, int l2, typename r>
struct Label<label_min_of<Label<r>, Label<temp_label<l1, l2>>>>
{
  static constexpr auto resolve() { return Label{}; }
  static constexpr bool can_resolve() { return false; }
};

template <int l1, int l2, int r1, int r2>
struct Label<label_min_of<Label<temp_label<l1, l2>>, Label<temp_label<r1, r2>>>>
{
  static constexpr auto resolve() { return Label{}; }
  static constexpr bool can_resolve() { return false; }
};

template <typename l1, typename l2, typename r>
struct Label<label_min_of<Label<label_min_of<l1, l2>>, Label<r>>>
{
  static constexpr bool can_resolve() { return Label<label_min_of<l1, l2>>::can_resolve(); }

  static constexpr auto resolve(std::true_type*) { return Label<label_min_of<DECT(Label<label_min_of<l1, l2>>::resolve()), Label<r>>>::resolve(); }
  static constexpr auto resolve(std::false_type*) { return Label{}; }
  static constexpr auto resolve()
  {
    std::integral_constant<bool, can_resolve()>* choice{ nullptr };
    return resolve(choice);
  }
};

template <typename l1, typename l2, typename r>
struct Label<label_min_of<Label<r>, Label<label_min_of<l1, l2>>>>
{
  static constexpr bool can_resolve() { return Label<label_min_of<l1, l2>>::can_resolve(); }
  static constexpr auto resolve(std::true_type*) { return Label<label_min_of<DECT(Label<label_min_of<l1, l2>>::resolve()), Label<r>>>::resolve(); }

  static constexpr auto resolve(std::false_type*) { return Label{}; }

  static constexpr auto resolve()
  {
    std::integral_constant<bool, can_resolve()>* choice{ nullptr };
    return resolve(choice);
  }
};

template <typename l1, typename l2, int r1, int r2>
struct Label<label_min_of<Label<label_min_of<l1, l2>>, Label<temp_label<r1, r2>>>>
{
  static constexpr bool can_resolve() { return Label<label_min_of<l1, l2>>::can_resolve(); }

  static constexpr auto resolve(std::true_type*)
  {
    return Label<label_min_of<DECT(Label<label_min_of<l1, l2>>::resolve()), Label<temp_label<r1, r2>>>>::resolve();
  }
  static constexpr auto resolve(std::false_type*) { return Label{}; }
  static constexpr auto resolve()
  {
    std::integral_constant<bool, can_resolve()>* choice{ nullptr };
    return resolve(choice);
  }
};

template <typename l1, typename l2, int r1, int r2>
struct Label<label_min_of<Label<temp_label<r1, r2>>, Label<label_min_of<l1, l2>>>>
{
  static constexpr bool can_resolve() { return Label<label_min_of<l1, l2>>::can_resolve(); }
  static constexpr auto resolve(std::true_type*)
  {
    return Label<label_min_of<DECT(Label<label_min_of<l1, l2>>::resolve()), Label<temp_label<r1, r2>>>>::resolve();
  }

  static constexpr auto resolve(std::false_type*) { return Label{}; }

  static constexpr auto resolve()
  {
    std::integral_constant<bool, can_resolve()>* choice{ nullptr };
    return resolve(choice);
  }
};

template <typename l1, typename l2, typename r1, typename r2>
struct Label<label_min_of<Label<label_min_of<l1, l2>>, Label<label_min_of<r1, r2>>>>
{
  static constexpr bool can_resolve() { return Label<label_min_of<l1, l2>>::can_resolve() || Label<label_min_of<r1, r2>>::can_resolve(); }
  static constexpr auto resolve(std::true_type*)
  {
    return Label<label_min_of<DECT(Label<label_min_of<l1, l2>>::resolve()), DECT(Label<label_min_of<r1, r2>>::resolve())>>::resolve();
  }
  static constexpr auto resolve(std::false_type*) { return Label{}; }

  static constexpr auto resolve()
  {
    std::integral_constant<bool, can_resolve()>* choice{ nullptr };
    return resolve(choice);
  }
};

template <typename l, typename r>
struct Label<label_min_of<Label<l>, Label<r>>>
{
  static constexpr auto resolve() { return Label<l>::min_with(Label<r>{}); }
};

struct bottom;
template <>
struct Label<bottom>
{

  using is_label = std::true_type;

  constexpr Label() = default;

  template <typename T>
  constexpr static bool flows_to(const Label<T>&)
  {
    return std::is_same<Label, Label<T>>::value;
  }

  template <typename... T2>
  constexpr static auto min_with(const T2&... )
  {
    return Label{};
  }

  template <typename... labels>
  constexpr static bool is_min_of(const labels&...)
  {
    return true;
  }

  template <typename... labels>
  constexpr static bool is_max_of(const labels&...)
  {
    return (true && ... && Label::flows_to(labels{}));
  }

	using requires_causal_tracking = std::false_type;
	using can_abort = std::false_type;
	
};

template <typename l, typename r>
using resolved_label_min =
  std::conditional_t<std::is_same<l, Label<bottom>>::value || std::is_same<r, Label<bottom>>::value, Label<bottom>, DECT(Label<label_min_of<l, r>>::resolve())>;

template <typename L1, typename L2>
using label_lte = std::integral_constant<bool, L2::flows_to(L1{})>;

	std::ostream& operator<<(std::ostream& o, const Label<top>&);
template <typename... labels>
std::ostream& operator<<(std::ostream& o, const Label<label_min_of<labels...>>&)
{
  static const auto print = [&](const auto& e) {
    o << e << ",";
    return nullptr;
  };
  o << "min(";
  auto ignore = { print(labels{})... };
  return o << ")";
}

template <int seq, int depth>
std::ostream& operator<<(std::ostream& o, const Label<temp_label<seq, depth>>&)
{
  return o << "temp<" << seq << depth << ">";
}

	std::ostream& operator<<(std::ostream& o, const Label<bottom>&);
}
