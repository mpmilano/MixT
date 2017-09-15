#pragma once


namespace myria {
namespace mtl {
namespace split_phase {

template <typename... R>
struct requires;

template <typename... P>
struct provides;

template <typename... I>
struct inherits;

template <typename label, typename ast, typename oldinherit, typename... available>
constexpr auto collect_phase(ast, provides<available...> /*previous provides*/, oldinherit);

#define BUILD_VARIABLE_BINDING_LIST(requires)                                                                                                                  \
  template <typename... R>                                                                                                                                     \
  struct requires                                                                                                                                              \
  {                                                                                                                                                            \
    constexpr requires() = default;                                                                                                                            \
    template <typename T>                                                                                                                                      \
    static constexpr auto add(const T&)                                                                                                                        \
    {                                                                                                                                                          \
      return requires<R..., T>{};                                                                                                                              \
    }                                                                                                                                                          \
    template <typename... other>                                                                                                                               \
    static constexpr auto combine(requires<other...>)                                                                                                          \
    {                                                                                                                                                          \
      return requires<R..., other...>{};                                                                                                                       \
    }                                                                                                                                                          \
    template <typename T>                                                                                                                                      \
    static constexpr auto contains()                                                                                                                           \
    {                                                                                                                                                          \
      return mutils::contained<T, R...>();                                                                                                                     \
    }                                                                                                                                                          \
    template <typename... t2>                                                                                                                                  \
    static constexpr auto addAll(mutils::typelist<t2...>)                                                                                                      \
    {                                                                                                                                                          \
      return requires<R..., t2...>{};                                                                                                                          \
    }                                                                                                                                                          \
    using as_typelist = mutils::typelist<R...>;                                                                                                                \
    template <typename T>                                                                                                                                      \
    using find_subtype = typename as_typelist::template find_subtype<T>;                                                                                       \
    template <typename S>                                                                                                                                      \
    static constexpr bool contains_subtype()                                                                                                                   \
    {                                                                                                                                                          \
      return as_typelist::template contains_subtype<S>();                                                                                                      \
    }                                                                                                                                                          \
    static_assert(mutils::forall<is_variable_binding<R>::value...>());                                                                                         \
  };                                                                                                                                                           \
                                                                                                                                                               \
  template <typename>                                                                                                                                          \
  struct is_##requires;                                                                                                                                        \
  template <typename... R>                                                                                                                                     \
  struct is_##requires<requires<R...>> : public std::true_type                                                                                                 \
  {                                                                                                                                                            \
  };                                                                                                                                                           \
  template <typename>                                                                                                                                          \
  struct is_##requires : public std::false_type                                                                                                                \
  {                                                                                                                                                            \
  };                                                                                                                                                           \
  template <typename a, typename b>                                                                                                                            \
  using combined_##requires = DECT(a::combine(b{}));                                                                                                           \
  template <typename... bindings>                                                                                                                              \
  constexpr auto to_##requires(mutils::typeset<bindings...>)                                                                                                   \
  {                                                                                                                                                            \
    return requires<bindings...>{};																			\
  }

template <typename, typename, typename, typename>
struct phase_api;

template <typename, typename, typename, typename>
struct extracted_phase;
}
}
}
