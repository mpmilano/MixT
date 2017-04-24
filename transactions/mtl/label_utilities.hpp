#pragma once

namespace myria{
template <typename>
struct Label;
  namespace labels{
  
  template<typename l>
  constexpr static auto min_of(const Label<l>& lbl){
    return lbl;
  }
  
  template<typename l1, typename l3, typename... l2>
  constexpr static auto min_of(const Label<l1>&, const l3&, const l2&... ){
    constexpr bool this_is_min = (l3::flows_to(Label<l1>{}) && ... && l2::flows_to(Label<l1>{}));
    constexpr bool min_lies_in_rest = (Label<l1>::flows_to(l3{}) || ... || Label<l1>::flows_to(l2{}));
    static_assert(this_is_min || min_lies_in_rest,
		  "Error min of labels not contained within arguments; need abstracted meet/join to fulfill");
    return std::conditional_t<min_lies_in_rest, DECT(min_of(l3{},l2{}...)),
			      std::conditional_t<this_is_min, Label<l1>, mutils::mismatch >
			      >{};
  }

  template<typename l>
  constexpr static auto max_of(const Label<l>& lbl){
    return lbl;
  }
  
  template<typename l1, typename l3, typename... l2>
  constexpr static auto max_of(const Label<l1>&, const l3&, const l2&... ){
    constexpr bool this_is_max = (Label<l1>::flows_to(l3{}) && ... && Label<l1>::flows_to(l2{}));
    constexpr bool max_lies_ax_rest = (l3::flows_to(Label<l1>{}) || ... || l2::flows_to(Label<l1>{}));
    static_assert(this_is_max || max_lies_ax_rest,
		  "Error max of labels not contaaxed withax arguments; need abstracted meet/joax to fulfill");
    return std::conditional_t<max_lies_ax_rest, DECT(max_of(l3{},l2{}...)),
			      std::conditional_t<this_is_max, Label<l1>, mutils::mismatch >
			      >{};
  }

  }

  template<typename l1>
  constexpr bool operator==(const Label<l1>&, const Label<l1>&){
    return true;
  }

  template<typename l1, typename l2>
  constexpr std::enable_if_t<!std::is_same<l1,l2>::value,bool>
  operator==(const Label<l1>&, const Label<l2>&){
    return false;
  }

  template<char... chr>
  constexpr auto parse_label(mutils::String<chr...> s){
    return Label<DECT(s)>{};
  }

  template<char... chr>
  std::ostream& operator<<(std::ostream& o, const Label<mutils::String<chr...> >&){
    return o << mutils::String<chr...>{};
  }
  
}
