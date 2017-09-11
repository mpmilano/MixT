#pragma once
#include "mtlutils.hpp"
#include "label_utilities.hpp"

namespace myria {
template <typename>
struct Label;
	using top = mutils::String<'t','o','p'>;
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

  using might_track = std::false_type;
  template <typename>
  using tracks_against = std::false_type;
  using run_remotely = std::false_type;
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
	
  static constexpr auto resolve() {
    return labels::min_of(Label<l>{}, Label<r>{}); }
};

using bottom = mutils::String<'b','o','t','t','o','m'>;
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

  using might_track = std::false_type;
  template <typename>
  using tracks_against = std::false_type;
  using run_remotely = std::false_type;
  using can_abort = std::false_type;
};

template <typename l, typename r>
using resolved_label_min =
  std::conditional_t<std::is_same<l, Label<bottom>>::value || std::is_same<r, Label<bottom>>::value, Label<bottom>, DECT(Label<label_min_of<l, r>>::resolve())>;

template<typename L1, typename L2>
constexpr auto* resolved_label_min_vararg_f(L1*, L2*){
  constexpr resolved_label_min<L1,L2> *np{nullptr};
  return np;
}

template<typename L1, typename L2, typename L3, typename... Lr>
constexpr auto* resolved_label_min_vararg_f(L1* a, L2* b, L3* c, Lr*... d){
  return resolved_label_min_vararg_f(resolved_label_min_vararg_f(a,b),c,d...);
}
    
template<typename... l>
using resolved_label_min_vararg = DECT(*resolved_label_min_vararg_f(std::declval<l*>()... ));
	
template<typename L1, typename L2>
constexpr auto* label_min_vararg_f(L1*, L2*){
	constexpr Label<label_min_of<L1,L2> > *np{nullptr};
	return np;
}

template<typename L1, typename L2, typename L3, typename... Lr>
constexpr auto* label_min_vararg_f(L1* a, L2* b, L3* c, Lr*... d){
  return label_min_vararg_f(label_min_vararg_f(a,b),c,d...);
}
	
template<typename... l>
using label_min_vararg = DECT(*label_min_vararg_f(std::declval<l*>()... ));

template <typename L1, typename L2>
using label_lte = std::integral_constant<bool, L2::flows_to(L1{})>;

	template<typename label>
constexpr auto print_comma_separated(label);

template <char... c>
auto print_label(const Label<mutils::String<c...>>&)
{
	return mutils::String<c...>{};
}

template <typename l1, typename l2>
auto print_label(const Label<label_min_of<l1, l2> >&)
{
	using namespace mutils;
	return String<'m','i','n','('>::append(print_comma_separated(l1{},l2{}).template append<')'>());
}

template <int seq, int depth>
auto print_label(const Label<temp_label<seq, depth>>&)
{
	using namespace mutils;
	return String<'t','e','m','p','<'>::append(string_from_int<seq>())
		.append(string_from_int<depth>()).append(String<'>'>{});
}

	template<typename label>
	constexpr auto print_comma_separated(label){
		return print_label(label{}).template append<','>();
	}

	template<typename l>
	std::ostream& operator<<(std::ostream& o, const Label<l>& a){
		return o << print_label(a);
	}

}
