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
template <typename real, typename l, typename r>
struct label_min_of2;
template <typename real, typename l, typename r>
struct label_min_of4;
	
template <typename real, int l1, int l2, typename r>
struct label_min_of4<real,Label<temp_label<l1, l2>>, Label<r>>
{
  static constexpr auto resolve() { return real{}; }
};

template <typename real, int l1, int l2, typename r>
struct label_min_of4<real,Label<r>, Label<temp_label<l1, l2> > >
{
  static constexpr auto resolve() { return real{}; }
};

template <typename real, int l1, int l2, int r1, int r2>
struct label_min_of4<real,Label<temp_label<l1, l2>>, Label<temp_label<r1, r2>>>
{
  static constexpr auto resolve() { return real{}; }
};

template <typename real, typename l1, typename l2, typename r>
struct label_min_of4<real,Label<label_min_of<l1, l2>>, Label<r>>
{
	static constexpr auto resolve()
  {
		using candidate = Label<label_min_of<DECT(Label<label_min_of<l1, l2>>::resolve()), Label<r>>>;
		if constexpr (std::is_same<candidate,real>::value) return real{};
    else return candidate::resolve();
  }
};

template <typename real, typename l1, typename l2, typename r>
struct label_min_of4<real,Label<r>, Label<label_min_of<l1, l2>>>
{

  static constexpr auto resolve()
  {
		using candidate = Label<label_min_of<DECT(Label<label_min_of<l1, l2>>::resolve()), Label<r>>>;
		if constexpr (std::is_same<candidate,real>::value) return real{};
    else return candidate::resolve();
  }
};

template <typename real, typename l1, typename l2, int r1, int r2>
struct label_min_of4<real,Label<label_min_of<l1, l2>>, Label<temp_label<r1, r2>>>
{
  static constexpr auto resolve()
  {
		using candidate = Label<label_min_of<DECT(Label<label_min_of<l1, l2>>::resolve()), Label<temp_label<r1, r2>>>>;
		if constexpr (std::is_same<candidate,real>::value) return real{};
    else return candidate::resolve();
  }
};

template <typename real, typename l1, typename l2, int r1, int r2>
struct label_min_of4<real,Label<temp_label<r1, r2>>, Label<label_min_of<l1, l2>>>
{

	static constexpr auto resolve()
  {
    using candidate = Label<label_min_of<DECT(Label<label_min_of<l1, l2>>::resolve()), Label<temp_label<r1, r2>>>>;
		if constexpr (std::is_same<candidate,real>::value) return real{};
    else return candidate::resolve();
  }
};

template <typename real, typename l1, typename l2, typename r1, typename r2>
struct label_min_of4<real,Label<label_min_of<l1, l2>>, Label<label_min_of<r1, r2>>>
{
  static constexpr auto resolve()
  {
    using candidate =  Label<label_min_of<DECT(Label<label_min_of<l1, l2>>::resolve()), DECT(Label<label_min_of<r1, r2>>::resolve())>>;
		if constexpr (std::is_same<candidate,real>::value) return real{};
		else return candidate::resolve();
  }

};
	
template <typename real, typename l, typename r>
struct label_min_of2 : public label_min_of4<real,l, r>
{
	using label_min_of4<real,l, r>::resolve;
};

template <typename real, typename l>
struct label_min_of2<real,l, Label<top> >
{
  static constexpr auto resolve() { return l{}; }
};

template <typename real, typename r>
struct label_min_of2<real,Label<top>, r >
{
  static constexpr auto resolve() { return r{}; }
};

template <typename real>
struct label_min_of2<real,Label<top>, Label<top> >
{
  static constexpr auto resolve() { return Label<top>{}; }
};

template <typename l, typename r>
struct Label<label_min_of<l, r> > : public label_min_of2<Label<label_min_of<l, r> >, l, r>
{
	using label_min_of2<Label,l, r>::resolve;
};

template <typename l>
struct Label<label_min_of<l, Label<bottom> > >
{
  static constexpr auto resolve() { return Label<bottom>{}; }
};

template <typename r>
struct Label<label_min_of<Label<bottom>, r > >
{
  static constexpr auto resolve() { return Label<bottom>{}; }
};

template <>
struct Label<label_min_of<Label<bottom>, Label<bottom> > >
{
  static constexpr auto resolve() { return Label<bottom>{}; }
};

template <typename l, typename r>
using resolved_label_min = DECT(Label<label_min_of<l, r>>::resolve());

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
	
template <typename... labels>
auto print_label(const Label<label_min_of<labels...>>&)
{
	using namespace mutils;
	return String<'m','i','n','('>::append(print_comma_separated(labels{})...).template append<')'>();
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
