#pragma once
#include "CTString_decl.hpp"

#define DECT(x...) std::decay_t<decltype(x)>

namespace mutils {

template <typename...>
struct typelist;

template <bool... b>
constexpr static bool
forall()
{
  return (true && ... && b);
}

template <bool... b>
constexpr static bool
exists()
{
  return (false || ... || b);
}

template <typename Cand, typename... List>
constexpr bool
contained()
{
  return exists<std::is_same<Cand, List>::value...>();
}

struct mismatch;

template <typename S>
constexpr auto*
find_subtype_f()
{
  constexpr mismatch* np{ nullptr };
  return np;
}

template <typename S, typename T1, typename... T>
constexpr auto* find_subtype_f(std::enable_if_t<!std::is_base_of<S, T1>::value>* = nullptr);

template <typename S, typename T1, typename... T>
constexpr auto*
find_subtype_f(std::enable_if_t<std::is_base_of<S, T1>::value>* = nullptr)
{
  constexpr T1* np{ nullptr };
  return np;
}

template <typename S, typename T1, typename... T>
constexpr auto*
find_subtype_f(std::enable_if_t<!std::is_base_of<S, T1>::value>*)
{
  return find_subtype_f<S, T...>();
}

template <typename S, typename... T>
using find_subtype = DECT(*find_subtype_f<S, T...>());

template <typename S>
constexpr bool
contains_subtype()
{
  return false;
}

template <typename S, typename T1, typename... T>
constexpr bool
contains_subtype()
{
  return std::is_base_of<S, T1>::value || contains_subtype<S, T...>();
}

namespace typelist_ns {

template <template <typename, typename> class less_than, typename... list>
constexpr auto min();

template <template <typename, typename> class less_than, typename... rst>
constexpr auto min_helper();

template <template <typename, typename> class less_than, typename fst>
constexpr auto _min_helper()
{
  constexpr fst* np{ nullptr };
  return np;
}

template <template <typename, typename> class less_than, typename candidate, typename fst, typename... list>
constexpr auto _min_helper(std::enable_if_t<!less_than<fst, candidate>::value>* = nullptr)
{
  return min_helper<less_than, candidate, list...>();
}

template <template <typename, typename> class less_than, typename candidate, typename fst, typename... list>
constexpr auto _min_helper(std::enable_if_t<less_than<fst, candidate>::value>* = nullptr)
{
  return min_helper<less_than, fst, list...>();
}

template <template <typename, typename> class less_than, typename... rst>
constexpr auto min_helper()
{
  return _min_helper<less_than, rst...>();
}

template <template <typename, typename> class less_than, typename fst, typename... list>
constexpr auto _min()
{
  return min_helper<less_than, fst, list...>();
}

template <typename cand>
constexpr auto remove_first(const typelist<>& a)
{
  return a;
}

template <typename cand, typename fst, typename... rst>
constexpr auto remove_first(typelist<fst, rst...>, std::enable_if_t<!std::is_same<cand, fst>::value>* = nullptr);

template <typename cand, typename fst, typename... rst>
constexpr auto remove_first(typelist<fst, rst...>, std::enable_if_t<std::is_same<cand, fst>::value>* = nullptr)
{
  return typelist<rst...>{};
}

template <typename cand, typename fst, typename... rst>
constexpr auto remove_first(typelist<fst, rst...>, std::enable_if_t<!std::is_same<cand, fst>::value>*)
{
  return typelist<fst>::append(remove_first<cand>(typelist<rst...>{}));
}
template <template <typename, typename> class less_than, typename... list>
constexpr auto min()
{
  return _min<less_than, list...>();
}

template <typename c1, typename... str2>
constexpr auto reverse_helper()
{
  return typelist<str2...>::reverse().append(typelist<c1>{});
}
}

template <>
struct typelist<>
{
  constexpr typelist() = default;
  template <typename... t2>
  static constexpr auto append(typelist<t2...> a)
  {
    return a;
  }

  template <typename ap1, typename ap2, typename... rest>
  static constexpr auto append(ap1, ap2, rest...)
  {
    return append(ap1{}).append(ap2{}).append(rest{}...);
  }

  static constexpr auto append() { return typelist{}; }

  template <typename T>
  static constexpr bool contains()
  {
    return false;
  }

  template <template <typename, typename> class less_than>
  static constexpr auto sort()
  {
    return typelist{};
  }

  static constexpr auto reverse() { return typelist{}; }

  template <typename... T>
  static constexpr auto intersect(T...)
  {
    return typelist{};
  }

  template <typename>
  using find_subtype = mismatch;

  template <typename>
  static constexpr bool contains_subtype()
  {
    return false;
  }
};

namespace typelist_ns {
constexpr auto append()
{
  return typelist<>{};
}
template <typename T, typename... U>
constexpr auto append(T, U... u)
{
  return T::append(u...);
}
}

template <typename... t1>
struct typelist
{
  constexpr typelist() = default;
  template <typename... t2>
  static constexpr auto append(typelist<t2...>)
  {
    return typelist<t1..., t2...>{};
  }

  template <typename ap1, typename ap2, typename... rest>
  static constexpr auto append(ap1, ap2, rest...)
  {
    return append(ap1{}).append(ap2{}).append(rest{}...);
  }

  static constexpr auto append() { return typelist{}; }

  template <typename T>
  static constexpr bool contains()
  {
    return contained<T, t1...>();
  }

  template <template <typename, typename> class less_than>
  static constexpr auto sort()
  {
    using fst = DECT(*typelist_ns::min<less_than, t1...>());
    return typelist<fst>::append(typelist_ns::remove_first<fst>(typelist{}).template sort<less_than>());
  }

  static constexpr auto reverse() { return typelist_ns::reverse_helper<t1...>(); }

private:
  template <typename sofar>
  static constexpr auto intersect_helper(sofar a, typelist<>)
  {
    return a;
  }

  template <typename sofar, typename t, typename... t2>
  static constexpr auto intersect_helper(sofar, typelist<t, t2...>)
  {
    return typelist::intersect_helper(std::conditional_t<contains<t>(), DECT(sofar::append(typelist<t>{})), sofar>{}, typelist<t2...>{});
  }

public:
  template <typename... t2>
  static constexpr auto intersect(typelist<t2...> a)
  {
    return typelist::intersect_helper<typelist<>, t2...>(typelist<>{}, a);
  }

  template <typename S>
  using find_subtype = ::mutils::find_subtype<S, t1...>;

  template <typename S>
  static constexpr bool contains_subtype()
  {
    return ::mutils::contains_subtype<S, t1...>();
  }
};

template <typename T>
struct string_of
{
  const std::string value;
  string_of()
    : value([] {
      std::stringstream ss;
      ss << T{};
      return ss.str();
    }())
  {
  }
};

template <typename... t>
std::ostream& operator<<(std::ostream& o, typelist<t...>)
{
  static const auto print = [&](const auto& e) {
    o << string_of<DECT(*e)>{}.value << ", ";
    return nullptr;
  };
  o << "[";
  auto ignore = { nullptr, nullptr, print((t*)nullptr)... };
  return o << "]";
}

template <typename... t1>
struct typeset;

template <typename... T2>
constexpr auto to_typeset(typelist<T2...>);

namespace typelist_ns {

template <typename typeset2, typename... t>
constexpr auto subtract(typeset<t...>, typeset2);
}

template <typename... t1>
struct typeset
{
  constexpr typeset() = default;

  template <typename t>
  static constexpr auto add(std::enable_if_t<contained<t, t1...>()>* = nullptr)
  {
    return typeset{};
  }

  template <typename t>
  static constexpr auto add(std::enable_if_t<!contained<t, t1...>()>* = nullptr)
  {
    return typeset<t1..., t>{};
  }

  template <typename... t>
  static constexpr auto add(typelist<t...>* = nullptr, std::enable_if_t<sizeof...(t) == 0>* = nullptr)
  {
    return typeset{};
  }

  template <typename a, typename b, typename... rst>
  static constexpr auto add()
  {
    return add<a>().template add<b>().template add<rst...>();
  }

  template <typename... t2>
  static constexpr auto combine(typeset<t2...>)
  {
    return typeset::template add<t2...>();
  }

  template <typename t>
  static constexpr bool contains()
  {
    return typelist<t1...>::template contains<t>();
  }

  template <typename ap1, typename ap2, typename... rest>
  static constexpr auto combine(ap1, ap2, rest...)
  {
    return combine(ap1{}).combine(ap2{}).combine(rest{}...);
  }

  static constexpr auto combine() { return typeset{}; }

  template <template <typename, typename> class less_than>
  static constexpr auto as_sorted_list()
  {
    return typelist<t1...>::template sort<less_than>();
  }

  template <typename... t2>
  static constexpr auto intersect(typeset<t2...>)
  {
    return to_typeset(typelist<t1...>::intersect(typelist<t2...>{}));
  }

  template <typename... t2>
  static constexpr auto subtract(typeset<t2...>)
  {
    return typelist_ns::subtract(typeset{}, typeset<t2...>{});
  }
};

namespace typelist_ns {

constexpr auto combine()
{
  return typeset<>{};
}
template <typename T, typename... U>
constexpr auto combine(T, U... u)
{
  return T::combine(u...);
}

template <typename typeset2, typename accum>
constexpr auto _subtract(accum a, typeset<>, typeset2)
{
  return a;
}

template <typename typeset2, typename accum, typename t1, typename... t>
constexpr auto _subtract(accum, typeset<t1, t...>, typeset2 b, std::enable_if_t<!typeset2::template contains<t1>()>* = nullptr);

template <typename typeset2, typename accum, typename t1, typename... t>
constexpr auto _subtract(accum, typeset<t1, t...>, typeset2 b, std::enable_if_t<typeset2::template contains<t1>()>* = nullptr)
{
  return _subtract(accum{}, typeset<t...>{}, b);
}

template <typename typeset2, typename accum, typename t1, typename... t>
constexpr auto _subtract(accum, typeset<t1, t...>, typeset2 b, std::enable_if_t<!typeset2::template contains<t1>()>*)
{
  return _subtract(accum::template add<t1>(), typeset<t...>{}, b);
}

template <typename typeset2, typename... t>
constexpr auto subtract(typeset<t...> a, typeset2 b)
{
  return _subtract(typeset<>{}, a, b);
}

constexpr typeset<>
intersect()
{
  return typeset<>{};
}

template <typename T>
constexpr T intersect(T)
{
  return T{};
}

template <typename T1, typename T2, typename... rest>
constexpr auto intersect(T1, T2, rest...)
{
  return T1::intersect(T2{}, rest{}...);
}
}

template <typename... t>
std::ostream& operator<<(std::ostream& o, typeset<t...>)
{
  static const auto print = [&](const auto& e) {
    o << string_of<DECT(*e)>{}.value << ", ";
    return nullptr;
  };
  o << "[";
  auto ignore = { nullptr, nullptr, print((t*)nullptr)... };
  return o << "]";
}

template <typename... T2>
constexpr auto to_typeset(typelist<T2...>)
{
  return typeset<>::add<T2...>();
}
/*
        template<typename T>
        constexpr bool print_obj(T = T{}){
                static_assert(T{} != T{});
                return true;
        }//*/

template <bool... b>
constexpr std::size_t
count_matches()
{
  return (0 + ... + b);
}

struct mismatch
{
  constexpr mismatch() = default;
};

template <typename... T>
constexpr bool
contains_match()
{
  return !forall<std::is_same<T, mismatch>::value...>();
}

template <typename... T>
constexpr bool
contains_single_match()
{
  return count_matches<std::is_same<T, mismatch>::value...>() == (sizeof...(T)-1);
}

template <typename K, typename... T>
constexpr K*
_find_match(std::enable_if_t<!std::is_same<K, mismatch>::value>* = nullptr)
{
  constexpr K* ret{ nullptr };
  return ret;
}

template <typename K, typename... T>
constexpr auto _find_match(std::enable_if_t<std::is_same<K, mismatch>::value>* = nullptr)
{
  return _find_match<T...>();
}

template <typename Fst, typename... T>
constexpr auto find_match()
{
  static_assert(contains_single_match<Fst, T...>());
  return _find_match<Fst, T...>();
}

constexpr bool
isDigit(char c)
{
  switch (c) {
    case '0':
      return true;
    case '1':
      return true;
    case '2':
      return true;
    case '3':
      return true;
    case '4':
      return true;
    case '5':
      return true;
    case '6':
      return true;
    case '7':
      return true;
    case '8':
      return true;
    case '9':
      return true;
    default:
      return false;
  };
}

constexpr unsigned char
toInt(char c)
{
  struct bad_parse
  {
  };
  switch (c) {
    case '0':
      return 0;
    case '1':
      return 1;
    case '2':
      return 2;
    case '3':
      return 3;
    case '4':
      return 4;
    case '5':
      return 5;
    case '6':
      return 6;
    case '7':
      return 7;
    case '8':
      return 8;
    case '9':
      return 9;
    default:
      throw bad_parse{};
  };
}

constexpr long long
exp(long long base, long long exponent)
{
  return (exponent == 0 ? 1 : base * exp(base, exponent - 1));
}

template <char a, char b>
void
print_varname(std::ostream& o, String<'a', 'n', 'o', 'r', 'm', a, b>)
{
  long long _a = a;
  long long _b = b;
  o << "anorm(" << _a << "," << _b << ")";
}

template <char a, char b>
void
print_varname(std::ostream& o, String<1, a, b>)
{
  long long _a = a;
  long long _b = b;
  o << "[" << _a << "," << _b << "]";
}

template <char a, char b>
void
print_varname(std::ostream& o, String<'w', 'h', 'i', 'l', 'e', a, b>)
{
  long long _a = a;
  long long _b = b;
  o << "while(" << _a << "," << _b << ")";
}

template <char... str>
void
print_varname(std::ostream& o, String<str...> s, std::enable_if_t<!String<str...>::begins_with(String<'a', 'n', 'o', 'r', 'm'>{})>* = nullptr)
{
  o << s;
}

template <typename K, typename V>
struct type_association_map_entry
{

  using key = K;
  using value = V;

  template <typename Other>
  static constexpr mutils::mismatch get(std::enable_if_t<!std::is_same<Other, K>::value>* = nullptr)
  {
    return mutils::mismatch{};
  };
  template <typename Key>
  static constexpr type_association_map_entry* get(std::enable_if_t<std::is_same<Key, K>::value>* = nullptr)
  {
    return nullptr;
  }
};

template <typename Key, typename map>
constexpr auto remove_entry();

template <typename... entries>
struct type_association_map;

template <>
struct type_association_map<>
{

  constexpr type_association_map() = default;

  template <typename Key>
  static constexpr bool contains()
  {
    return false;
  }

  template <typename Key>
  using without = type_association_map;

  template <typename Key, typename Value>
  using add_unconditional = type_association_map<type_association_map_entry<Key, Value>>;

  template <typename Key, typename Value>
  using add = add_unconditional<Key, Value>;
};

template <typename... entries>
struct type_association_map
{

  constexpr type_association_map() = default;

  template <typename Key>
  static constexpr bool contains()
  {
    return contained<Key, typename entries::key...>();
  }

  template <typename Key>
  using get = DECT(**find_match<DECT(entries::template get<Key>())...>());

  template <typename Key>
  using without = DECT(remove_entry<Key, type_association_map>());

  template <typename Key, typename Value>
  using add_unconditional = type_association_map<type_association_map_entry<Key, Value>, entries...>;

  template <typename Key, typename Value>
  using add = std::conditional_t<contains<Key>(), typename without<Key>::template add_unconditional<Key, Value>,
                                 type_association_map<entries..., type_association_map_entry<Key, Value>>>;
};

template <typename Key, typename Value>
struct string_of<type_association_map_entry<Key, Value>>
{
  std::string value = std::string{ string_of<Key>{}.value } + std::string{ " : " } + std::string{ string_of<Value>{}.value };
};

template <typename... entries>
std::ostream& operator<<(std::ostream& o, type_association_map<entries...>)
{
  return o << typelist<entries...>{};
}

template <typename Key, typename sofar>
constexpr auto remove_entry_f(sofar, type_association_map<>)
{
  return sofar{};
}
template <typename Key, typename sofar, typename Cand, typename Value, typename... entries>
constexpr auto remove_entry_f(sofar, type_association_map<type_association_map_entry<Cand, Value>, entries...>,
                              std::enable_if_t<!std::is_same<Cand, Key>::value>* = nullptr);
template <typename Key, typename sofar, typename Cand, typename Value, typename... entries>
constexpr auto remove_entry_f(sofar, type_association_map<type_association_map_entry<Cand, Value>, entries...>,
                              std::enable_if_t<std::is_same<Cand, Key>::value>* = nullptr)
{
  return remove_entry_f<Key>(sofar{}, type_association_map<entries...>{});
}
template <typename Key, typename sofar, typename Cand, typename Value, typename... entries>
constexpr auto remove_entry_f(sofar, type_association_map<type_association_map_entry<Cand, Value>, entries...>,
                              std::enable_if_t<!std::is_same<Cand, Key>::value>*)
{
  return remove_entry_f<Key>(typename sofar::template add_unconditional<Cand, Value>{}, type_association_map<entries...>{});
}

template <typename Key, typename map>
constexpr auto remove_entry()
{
  return remove_entry_f<Key>(type_association_map<>{}, map{});
}
}
