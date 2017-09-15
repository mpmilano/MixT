#pragma once
#include "mtl/environments.hpp"
#include "mtl/typecheck_and_label_decl.hpp"

namespace myria {
namespace mtl {
namespace split_phase {

template <char... str>
struct while_binding
{
  using holder = value_holder<std::size_t, str...>;
  using name = String<str...>;
  template <typename Other>
  static constexpr mutils::mismatch get_binding(std::enable_if_t<!std::is_same<Other, name>::value, Other>)
  {
    return mutils::mismatch{};
  }
  template <typename>
  static constexpr while_binding get_binding(name)
  {
    return while_binding{};
  }
  template <typename T2>
  static constexpr auto get_binding2(const T2& t)
  {
    return get_binding<T2>(t);
  }
};

template <typename>
struct is_while_binding;
template <char... str>
struct is_while_binding<while_binding<str...>> : public std::true_type
{
};
template <typename>
struct is_while_binding : public std::false_type
{
};

template <typename T>
using is_variable_binding = std::integral_constant<bool, (is_type_binding<T>::value || is_while_binding<T>::value)>;
	
BUILD_VARIABLE_BINDING_LIST(requires);
BUILD_VARIABLE_BINDING_LIST(provides);
BUILD_VARIABLE_BINDING_LIST(inherits);

template <typename _label, typename _requires, typename _provides, typename _inherits>
struct phase_api<Label<_label>, _requires, _provides, _inherits>
{
  constexpr phase_api() = default;
  using requires = _requires;
  using provides = _provides;
  using inherits = _inherits;
  using label = Label<_label>;
  static_assert(is_requires<requires>::value);
  static_assert(is_provides<provides>::value);
  static_assert(is_inherits<inherits>::value);
  template <typename T>
  static constexpr auto add_provides(const T&)
  {
    return phase_api<label, requires, DECT(provides::add(T{})), inherits>{};
  }

  template <typename T>
  static constexpr auto add_requires(const T&)
  {
    return phase_api<label, DECT(requires::add(T{})), provides, inherits>{};
  }
};

	template<typename, typename> struct combined_api_str;

template <typename label, typename req1, typename prov1, typename inherits, typename req2, typename prov2>
struct combined_api_str<phase_api<label, req1, prov1, inherits>, phase_api<label, req2, prov2, inherits>>
{
  using type = phase_api<label, combined_requires<req1, req2>, combined_provides<prov1, prov2>, inherits>;
};

	template<typename a>
	auto combined_api_f(a* _a){
		return _a;
	}
	
	template<typename a, typename b, typename... c>
	auto combined_api_f(a*,b*,c*... _c){
		constexpr typename combined_api_str<a,b>::type *recur{nullptr};
		return combined_api_f(recur,_c...);
	}

	template <typename... a>
	using combined_api = DECT(*combined_api_f( ((a*)nullptr)...));


}
}
}
