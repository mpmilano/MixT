#pragma once

#include "CTString_decl.hpp"
#include "CTString_impl.hpp"
#include "macro_utils.hpp"

namespace mutils {
namespace CTString {
template <char... str>
struct char_pack
{
  template <char str2>
  static constexpr char_pack<str2, str...>* prepend()
  {
    return nullptr;
  }
};

constexpr char_pack<>*
strip_nulls(char_pack<> const* const, void*)
{
  return nullptr;
}

template <char str1, char... str>
constexpr auto strip_nulls(char_pack<str1, str...> const* const, std::enable_if_t<str1 != 0>*)
{
  char_pack<str...>* rec_pack{ nullptr };
  auto pack1 = strip_nulls(rec_pack, nullptr);
  using cp1_t = std::decay_t<decltype(*pack1)>;
  return cp1_t::template prepend<str1>();
}

template <char str1, char... str>
constexpr auto strip_nulls(char_pack<str1, str...> const* const, std::enable_if_t<str1 == 0>*)
{
  char_pack<str...>* rec_pack{ nullptr };
  return strip_nulls(rec_pack, nullptr);
}

template <char... str>
constexpr String<str...>*
string_from_stripped(char_pack<str...> const* const)
{
  return nullptr;
}

template <char... str>
constexpr auto string_from_macro_f()
{
  char_pack<str...>* rec_pack{ nullptr };
  auto ptr = strip_nulls(rec_pack, nullptr);
  return string_from_stripped(ptr);
}

template <char... str>
using string_from_macro = std::decay_t<decltype(*string_from_macro_f<str...>())>;

#define MUTILS_STRING1(str) ::mutils::CTString::string_from_macro<MACRO_GET_STR(#str)>
#define MUTILS_STRING2(str1, str2)                                                                                                                             \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}).append(MUTILS_STRING1(str2){}))>
#define MUTILS_STRING3(str1, str2, str3)                                                                                                                       \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}).append(MUTILS_STRING2(str2, str3){}))>
#define MUTILS_STRING4(str1, str2, str3, s4)                                                                                                                   \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}).append(MUTILS_STRING3(str2, str3, s4){}))>
#define MUTILS_STRING5(str1, str2, str3, s4, s5)                                                                                                               \
  std::decay_t<decltype(                                                                                                                                       \
    ::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}).append(MUTILS_STRING4(str2, str3, s4, s5){}))>
#define MUTILS_STRING6(str1, str2, str3, s4, s5, s6)                                                                                                           \
  std::decay_t<decltype(                                                                                                                                       \
    ::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}).append(MUTILS_STRING5(str2, str3, s4, s5, s6){}))>
#define MUTILS_STRING7(str1, str2, str3, s4, s5, s6, s7)                                                                                                       \
  std::decay_t<decltype(                                                                                                                                       \
    ::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}).append(MUTILS_STRING6(str2, str3, s4, s5, s6, s7){}))>
#define MUTILS_STRING8(str1, str2, str3, s4, s5, s6, s7, s8)                                                                                                   \
  std::decay_t<decltype(                                                                                                                                       \
    ::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}).append(MUTILS_STRING7(str2, str3, s4, s5, s6, s7, s8){}))>
#define MUTILS_STRING9(str1, str2, str3, s4, s5, s6, s7, s8, s9)                                                                                               \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{})                                           \
                          .append(MUTILS_STRING8(str2, str3, s4, s5, s6, s7, s8, s9){}))>
#define MUTILS_STRING10(str1, str2, str3, s4, s5, s6, s7, s8, s9, s10)                                                                                         \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{})                                           \
                          .append(MUTILS_STRING9(str2, str3, s4, s5, s6, s7, s8, s9, s10){}))>
#define MUTILS_STRING11(str1, str2, str3, s4, s5, s6, s7, s8, s9, s10,s11)	\
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{})                                           \
			.append(MUTILS_STRING10(str2, str3, s4, s5, s6, s7, s8, s9, s10,s11){}))>
#define MUTILS_STRING12(str1, str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12) \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{})                                           \
			.append(MUTILS_STRING11(str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12){}))>
#define MUTILS_STRING13(str1, str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13) \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}) \
			.append(MUTILS_STRING12(str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13){}))>
#define MUTILS_STRING14(str1, str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14) \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}) \
			.append(MUTILS_STRING13(str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14){}))>
#define MUTILS_STRING15(str1, str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14,s15) \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}) \
			.append(MUTILS_STRING14(str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14,s15){}))>
#define MUTILS_STRING16(str1, str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14,s15,s16) \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}) \
			.append(MUTILS_STRING15(str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14,s15,s16){}))>
#define MUTILS_STRING17(str1, str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14,s15,s16,s17) \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}) \
			.append(MUTILS_STRING16(str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14,s15,s16,s17){}))>
#define MUTILS_STRING18(str1, str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14,s15,s16,s17,s18) \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}) \
			.append(MUTILS_STRING17(str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14,s15,s16,s17,s18){}))>
#define MUTILS_STRING19(str1, str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14,s15,s16,s17,s18,s19) \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}) \
			.append(MUTILS_STRING18(str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14,s15,s16,s17,s18,s19){}))>
#define MUTILS_STRING20(str1, str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14,s15,s16,s17,s18,s19,s20) \
  std::decay_t<decltype(::mutils::CTString::string_from_macro<MACRO_GET_STR(#str1)>::append(::mutils::String<','>{}) \
			.append(MUTILS_STRING19(str2, str3, s4, s5, s6, s7, s8, s9, s10,s11,s12,s13,s14,s15,s16,s17,s18,s19,s20){}))>

#define MUTILS_STRING_IMPL2(count, ...) MUTILS_STRING##count(__VA_ARGS__)
#define MUTILS_STRING_IMPL(count, ...) MUTILS_STRING_IMPL2(count, __VA_ARGS__)
#define MUTILS_STRING(...) MUTILS_STRING_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)
}
}
