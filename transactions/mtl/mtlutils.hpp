#pragma once
#include "mutils/typelist.hpp"
#include "mutils/typeset.hpp"

namespace mutils {

constexpr auto
print_varname(String<'z', 'e', 'r', 'o', 0, 1>)
{
	return String<'z', 'e', 'r', 'o'>{};
}

template<char a, char b>
constexpr auto
print_varname(String<'r','e','m','o','t','e','_','b','o','u','n','d','_','t','m','p', 0, a,b>)
{
	return String<'r','e','m','o','t','e','_','b','o','u','n','d','_','t','m','p'>{};
}

constexpr auto
print_varname(String<'o', 'n', 'e', 0, 1>)
{
	return String<'o', 'n', 'e'>{};
}
	
template <char a, char b>
constexpr auto
print_varname(String<'a', 'n', 'o', 'r', 'm', a, b>)
{
	return String<'a','n','o','r','m','_'>::append(string_from_int<a>())
		.template append<'_'>().append(string_from_int<b>());
}

template <char a, char b>
constexpr auto
print_varname(String<1, a, b>)
{
	return String<'a','['>::append(string_from_int<a>())
		.append(String<','>{}).append(string_from_int<b>()).append(String<']'>{});
}

template <char a, char b>
constexpr auto
print_varname(String<'w', 'h', 'i', 'l', 'e', a, b>)
{
	return String<'w','h','i','l','e','_'>::append(string_from_int<a>()).template append<'_'>()
		.append(string_from_int<b>());
}

template <char... str>
constexpr auto
print_varname(String<str...> s, std::enable_if_t<!String<str...>::begins_with(String<'a', 'n', 'o', 'r', 'm'>{})>* = nullptr)
{
	return s;
}

	template<char c>
	constexpr auto print_operator(){
		constexpr String<c> cand;
		if constexpr (c == '=') {
				return cand.append(cand);
			}
		else return cand;
	}

template <typename a>
void
print_varname(std::ostream& o, a)
{
	o << print_varname(a{});
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


template<bool b, typename...> constexpr bool useful_static_assert(){
  static_assert(b);
  return b;
}

  template<typename, typename... > struct follows_in_sequence_str;
  
  template<typename T>
  struct follows_in_sequence_str<T>{
    using type = mismatch;
  };

  template<typename m1, typename m2>
  struct follows_in_sequence_str<m1,m2>{
    using type = mismatch;
  };
  
  template<typename match, typename arg1, typename arg2, typename... args>
  struct follows_in_sequence_str<match, arg1, arg2, args...>{
    using type = std::conditional_t<std::is_same<match,arg1>::value, arg2,
				    typename follows_in_sequence_str<match,arg2,args...>::type
				    >;
  };
  
  template<typename match, typename... args>
  using follows_in_sequence = typename follows_in_sequence_str<match,args...>::type;

	template<typename match, typename... args>
	using is_sequence_end = std::is_same<follows_in_sequence<match,args...>, mismatch >;

	template<typename> struct typename_str;

	template<> struct typename_str<bool> {
		static std::string f(){return "bool";}
	};

	template<typename T> struct typename_str<std::list<T> > {
		static std::string f(){
			std::stringstream ss;
			ss << "list<" << typename_str<T>::f() << ">";
			return ss.str();
		}
	};

	template<> struct typename_str<int> {
		static std::string f(){return "int";}
	};

	template<> struct typename_str<std::string> {
		static std::string f(){return "string";}
	};
	
}

#define MATCH_CONTEXT(NAME) struct NAME 
#define MATCHES(...) static constexpr auto match(const __VA_ARGS__ &)
#define RETURN(...) __VA_ARGS__* {constexpr __VA_ARGS__* ret{nullptr}; return ret;}
#define RETURNVAL(...) std::integral_constant<DECT(__VA_ARGS__), __VA_ARGS__>* {constexpr std::integral_constant<DECT(__VA_ARGS__), __VA_ARGS__>* ret{nullptr}; return ret;}
#define MATCH(name,...) DECT(*name ::match(std::declval< __VA_ARGS__ >()))
