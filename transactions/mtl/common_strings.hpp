#pragma once
#include "mutils/CTString.hpp"

namespace myria {
namespace mtl {

using lparen_s = mutils::String<'('>;
using rparen_s = mutils::String<')'>;
using lbracket_s = mutils::String<'{'>;
using rbracket_s = mutils::String<'}'>;
using var_s = mutils::String<'v', 'a', 'r', ' '>;
using remote_s = mutils::String<'r', 'e', 'm', 'o', 't', 'e', ' '>;
using while_s = mutils::String<'w', 'h', 'i', 'l', 'e'>;
using if_s = mutils::String<'i', 'f'>;
using comma_s = mutils::String<','>;
using space_s = mutils::String<' '>;
  using plus_s = mutils::String<'+',' '>;
using times_s = mutils::String<'*',' '>;
using div_s = mutils::String<'/',' '>;
using and_s = mutils::String<'&', '&'>;
using or_s = mutils::String<'|', '|'>;
using lt_s = mutils::String<'<',' '>;
using gt_s = mutils::String<'>',' '>;
using eq_s = mutils::String<'=', '='>;
using arrow_s = mutils::String<'-','>'>;
using deref_s = mutils::String<'*'>;
using minus_s = mutils::String<'-',' '>;
using return_s = mutils::String<'r','e','t','u','r','n'>;
using isValid_str = mutils::String<'i','s','V','a','l','i','d'>;
using endorse_str = mutils::String<'e','n','d','o','r','s','e'>;
	using ensure_str = mutils::String<'e','n','s','u','r','e'>;

	template<char> struct binop_string_str;
	
	template<> struct binop_string_str<'='> { using type = eq_s; };
	template<> struct binop_string_str<'|'> { using type = or_s; };
	template<> struct binop_string_str<'&'> { using type = and_s; };
	template<> struct binop_string_str<'+'> { using type = plus_s; };
	template<> struct binop_string_str<'*'> { using type = times_s; };
	template<> struct binop_string_str<'/'> { using type = div_s; };
	template<> struct binop_string_str<'<'> { using type = lt_s; };
	template<> struct binop_string_str<'>'> { using type = gt_s; };
	template<> struct binop_string_str<'-'> { using type = minus_s; };
	template<char c > using binop_string = typename binop_string_str<c>::type;
		
  
}
}
