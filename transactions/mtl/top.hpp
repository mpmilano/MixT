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

template <typename>
struct is_top;
template <>
struct is_top<Label<top> > : public std::true_type
{
};
template <>
struct is_top<top> : public std::true_type
{
};
template <typename>
struct is_top : public std::false_type
{
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

template <typename>
struct is_bottom;
template <>
struct is_bottom<Label<bottom> > : public std::true_type
{
};
template <>
struct is_bottom<bottom> : public std::true_type
{
};
template <typename>
struct is_bottom : public std::false_type
{
};

	template<typename> struct is_real_label;
	
	template<char... c>
	struct is_real_label<Label<mutils::String<c...> > > : public std::true_type{};

	template<typename >
	struct is_real_label : public std::false_type{};
	
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

template <typename tmps, typename rls>
struct label_min_of;

template <typename>
struct is_min_of;
template <typename l, typename r>
struct is_min_of<Label<label_min_of<l, r>>> : public std::true_type
{
};
template <typename l, typename r>
struct is_min_of<label_min_of<l, r>> : public std::true_type
{
};
template <typename>
struct is_min_of : public std::false_type
{
};

	template<typename l, typename r>
	constexpr auto min_real_labels(){
		if constexpr (l::flows_to(r{})){ return r{}; }
		else if constexpr (r::flows_to(l{})) {return l{};}
		else return mutils::String<'c','r','i','s','i','s','!'>{};
	}
	
template <typename tmps, typename rls>
struct Label<label_min_of<tmps,mutils::typeset<rls> > >{
	using _rls = mutils::typeset<rls>;
	
	template<typename t2, typename r2>
	static constexpr auto combine(Label<label_min_of<t2,mutils::typeset<r2> > >){
		return Label<label_min_of<DECT(tmps::combine(t2{})), mutils::typeset<DECT(min_real_labels<rls,r2>()) > > >{};
	}

	template<typename t2>
	static constexpr auto combine(Label<label_min_of<t2,mutils::typeset<> > >){
		return Label<label_min_of<DECT(tmps::combine(t2{})), _rls> >{};
	}

	template<char... c>
	static constexpr auto add(Label<mutils::String<c... > > r2){
		return Label<label_min_of<tmps, mutils::typeset<DECT(min_real_labels<rls,DECT(r2)>()) > > >{};
	}

	template<int l, int r>
	static constexpr auto add(Label<temp_label<l,r> > a){
		return Label<label_min_of<DECT(tmps::template add<DECT(a)>()),_rls> >{};
	}
};

template <typename tmps>
struct Label<label_min_of<tmps,mutils::typeset<> > >{
	using _rls = mutils::typeset<>;
	
	template<typename t2, typename r2>
	static constexpr auto combine(Label<label_min_of<t2,mutils::typeset<r2> > >){
		return Label<label_min_of<DECT(tmps::combine(t2{})), mutils::typeset<r2> > >{};
	}

	template<typename t2>
	static constexpr auto combine(Label<label_min_of<t2,mutils::typeset<> > >){
		return Label<label_min_of<DECT(tmps::combine(t2{})), _rls> >{};
	}

	template<char... c>
	static constexpr auto add(Label<mutils::String<c... > > r2){
		return Label<label_min_of<tmps, mutils::typeset<DECT(r2)> > >{};
	}

	template<int l, int r>
	static constexpr auto add(Label<temp_label<l,r> > a){
		return Label<label_min_of<DECT(tmps::template add<DECT(a)>()),_rls> >{};
	}
};


	template<typename l, typename r>
	constexpr auto resolved_label_min_f(){
		if constexpr (is_bottom<l>::value || is_bottom<r>::value){
				return l{};
			}
		else if constexpr(is_top<l>::value){
				return r{};
			}
		else if constexpr(is_top<r>::value){
				return l{};
			}
		else if constexpr (is_min_of<l>::value && is_min_of<r>::value){
				return l::combine(r{});
			}
		else if constexpr (is_min_of<l>::value){
				return l::add(r{});
			}
		else if constexpr (is_min_of<r>::value){
				return r::add(l{});
			}
		else if constexpr (is_real_label<l>::value && is_real_label<r>::value){
				return min_real_labels<l,r>();
			}
		else return Label<label_min_of<mutils::typeset<>, mutils::typeset<> > >::add(l{}).add(r{});
	}

template <typename l, typename r>
using resolved_label_min = DECT(resolved_label_min_f<l,r>());

constexpr auto* resolved_label_min_vararg_f(){
	constexpr Label<top> *np{nullptr};
	return np;
}
		
template<typename L1>
constexpr auto* resolved_label_min_vararg_f(L1* a){
	return a;
}
		
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
	auto print_label(const Label<label_min_of<mutils::typeset<labels...>, mutils::typeset<> > >&)
{
	using namespace mutils;
	return String<'m','i','n','('>::append(print_comma_separated(labels{})...).template append<')'>();
}
	
	template <typename r, typename... labels>
	auto print_label(const Label<label_min_of<mutils::typeset<labels...>, mutils::typeset<r> > >&)
{
	return print_label(Label<label_min_of<mutils::typeset<labels...,r>, mutils::typeset<> > >{});
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
