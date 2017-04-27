#pragma once
#include "parse_expressions_decl.hpp"
#include "common_strings.hpp"

namespace myria {
namespace mtl {

namespace parse_expressions {

using namespace mutils;

template <char op, char... str>
constexpr auto parse_binop(String<str...>)
{
  constexpr auto l = parse_expression(String<str...>::split(zero{}, String<op,' '>{}).trim_ends());
  constexpr auto r = parse_expression(String<str...>::split(one{}, String<op,' '>{}).trim_ends());
  return parse_phase::BinOp<op, std::decay_t<decltype(l)>, std::decay_t<decltype(r)>>{};
}

template <char... str>
constexpr bool
contains_operator()
{
  return String<str...>::contains_outside_parens(times_s{}) || String<str...>::contains_outside_parens(div_s{}) ||
         String<str...>::contains_outside_parens(minus_s{}) || String<str...>::contains_outside_parens(and_s{}) ||
         String<str...>::contains_outside_parens(or_s{}) || String<str...>::contains_outside_parens(lt_s{}) ||
         String<str...>::contains_outside_parens(gt_s{}) || String<str...>::contains_outside_parens(eq_s{}) ||
         String<str...>::contains_outside_parens(plus_s{});
}

	template<char c>
	constexpr bool is_method_char(){
		return (c >= 'A' && c <= 'Z')
			|| (c >= 'a' && c <= 'z')
			|| (c == '_' || c == ')')
			|| (c >= '0' && c <= '9');
	}

template <char... str>
constexpr bool
contains_invocation(String<str...> = String<str...>{},
										std::enable_if_t<!String<str...>::contains(String<'('>{})>* = nullptr)
{
	return false;
}
	
template <char... str>
constexpr bool
contains_invocation(String<str...> = String<str...>{},
										std::enable_if_t<String<str...>::contains(String<'('>{})>* = nullptr)
{
	constexpr auto paren_group = String<str...>::template next_paren_group<'(',')'>();
	constexpr char c = DECT(paren_group)::pre::reverse().trim_ends().string[0];
	return is_method_char<c>();
}

	static_assert(!contains_invocation<'3'>());

template <char... str>
constexpr bool
contains_fieldref()
{
  return String<str...>::contains_outside_parens(String<'.'>{});
}

  template<char... str> constexpr bool is_deref(){
    return String<str...>::trim_ends().begins_with(deref_s{});
  }
  template<char... str> constexpr bool contains_arrow(){
    return String<str...>::contains_outside_parens(arrow_s{});
  }

template <typename T>
auto strip_expression(parse_phase::Expression<T>)
{
  return T{};
}

// whitespace
  template <char, char... str>
constexpr auto _parse_expression(String<' ',str...>)
{
  return strip_expression(parse_expression(String<str...>::trim_ends()));
}
  
// parens
  template <char, char... str>
  constexpr auto _parse_expression(String<'(',str...>, std::enable_if_t<String<str...>::last_char() == String<')'>{}>* = nullptr)
{
  return strip_expression(parse_expression(String<str...>::trim_ends().remove_last_char()));
}

// variable reference
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!contains_invocation<str...>() && !is_deref<str...>() && !contains_arrow<str...>() && !contains_operator<str...>() && !contains_fieldref<str...>() && !(String<str...>{}.string[0] == '(') && !(String<str...>{}.string[0] == ' ') &&
																 !String<str...>::trim_ends().contains(String<' '>{}) && !String<str...>::trim_ends().is_number() && is_method_char<String<str...>{}.string[0]>(),
																 String<str...>>)
{
  return parse_phase::VarReference<std::decay_t<decltype(String<str...>::trim_ends())>>{};
}

// plain value
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!contains_invocation<str...>() && !is_deref<str...>() && !contains_arrow<str...>() && !contains_operator<str...>() && !contains_fieldref<str...>() && !(String<str...>{}.string[0] == '(') && !(String<str...>{}.string[0] == ' ') &&
                                                    !String<str...>::trim_ends().contains(String<' '>{}) && String<str...>::trim_ends().is_number(),
                                                  String<str...>>)
{
  return parse_phase::Constant<String<str...>::trim_ends().parseInt()>{};
}

  static_assert(contains_operator<'a', '.', 'b', ' ', '<', ' ', 'e'>());
// field reference
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!contains_invocation<str...>() && !is_deref<str...>() && !contains_arrow<str...>() && !contains_operator<str...>() && contains_fieldref<str...>() && !(String<str...>{}.string[0] == ' '), String<str...>>)
{
  constexpr auto name_str = String<str...>::before_lst(String<'.'>{}).trim_ends();
  constexpr auto field_str = String<str...>::after_lst(String<'.'>{}).trim_ends();
  using name = DECT(parse_expression(name_str));
  using field = DECT(field_str);
  return parse_phase::FieldReference<name, field>{};
}

// ptr field reference
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!contains_invocation<str...>() && !is_deref<str...>() && contains_arrow<str...>() && !contains_operator<str...>() && !(String<str...>{}.string[0] == ' '), String<str...>>)
{
  constexpr auto name_str = String<str...>::before_lst(arrow_s{}).trim_ends();
  constexpr auto field_str = String<str...>::after_lst(arrow_s{}).trim_ends();
  using name = DECT(parse_expression(name_str));
  using field = DECT(field_str);
  return parse_phase::FieldPointerReference<name, field>{};
}

	template<typename... args>
	struct add_operations_struct{

		//this is just isValid
		template<typename hndl>
		static constexpr auto add_operation_args(parse_phase::Operation<hndl,parse_phase::isValid_str>,
																						 String<>){
			return parse_phase::IsValid<hndl>{};
		}

		//no (further) arguments possible
		template<typename hndl,typename operations_str>
		static constexpr auto add_operation_args(parse_phase::Operation<hndl,operations_str,args...> a,
																						 String<>){
			return a;
		}

		//last argument
		template<typename hndl,typename operations_str,char c, char... str>
		static constexpr auto add_operation_args(parse_phase::Operation<hndl,operations_str,args...>,
																						 String<c,str...> arg,
																						 std::enable_if_t<!String<c,str...>::contains_outside_parens(String<','>{})>*
																						 = nullptr){
			return parse_phase::Operation<hndl,operations_str,args..., DECT(parse_expression(arg))>{};
		}

		//at least two arguments remain
		template<typename hndl,typename operations_str,char... str>
		static constexpr auto add_operation_args(parse_phase::Operation<hndl,operations_str,args...>,
																						 String<str...>,
																						 std::enable_if_t<String<str...>::contains_outside_parens(String<','>{})>*
																						 = nullptr){

			constexpr auto rest = String<str...>::after_fst(String<','>{});
			constexpr auto arg = parse_expression(String<str...>::split(zero{}, String<','>{}));
			return add_operations_struct<args...,DECT(arg)>::
				add_operation_args(parse_phase::Operation<hndl,operations_str,args...,DECT(arg)>{},rest);
		}
	};

// operation (including isValid)
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!contains_operator<str...>() && !is_deref<str...>() && contains_invocation<str...>() && !(String<str...>{}.string[0] == '(')
																 && !(String<str...>{}.string[0] == ' '), String<str...>>)
{
	using method_group = DECT(String<str...>::template next_paren_group<'(',')'>());
  using invocation_str = typename method_group::pre;
	constexpr auto hndl_str = invocation_str::before_lst(String<'.'>{}).trim_ends();
  constexpr auto operation_str = invocation_str::after_lst(String<'.'>{}).trim_ends();
  using hndl = DECT(parse_expression(hndl_str));
  return add_operations_struct<>::
		add_operation_args(parse_phase::Operation<hndl, DECT(operation_str)>{},
											 method_group::paren::
											 trim_ends().
											 remove_last_char().
											 remove_first_char().
											 trim_ends());
}//*/

// deref
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<is_deref<str...>(), String<str...>>)
{
  return parse_phase::Dereference<
    DECT
    (parse_expression
     (String<str...>::after_fst(deref_s{}).trim_ends()))>{};
}

//&& operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!is_deref<str...>() && String<str...>::contains_outside_parens(and_s{}) && !(String<str...>{}.string[0] == ' '), String<str...>>)
{
  return parse_binop<'&', str...>(String<str...>{});
}
//|| operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!is_deref<str...>() && String<str...>::contains_outside_parens(or_s{}) && !(String<str...>{}.string[0] == ' '), String<str...>>)
{
  return parse_binop<'|', str...>(String<str...>{});
}

//< operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!is_deref<str...>() && String<str...>::contains_outside_parens(lt_s{}) && !(String<str...>{}.string[0] == ' '), String<str...>>)
{
  return parse_binop<'<', str...>(String<str...>{});
}
//> operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!is_deref<str...>() && String<str...>::contains_outside_parens(gt_s{}) && !(String<str...>{}.string[0] == ' '), String<str...>>)
{
  return parse_binop<'>', str...>(String<str...>{});
}
//== operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!is_deref<str...>() && String<str...>::contains_outside_parens(eq_s{}) && !(String<str...>{}.string[0] == ' '), String<str...>>)
{
  return parse_binop<'=', str...>(String<str...>{});
}

//+ operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!is_deref<str...>() && String<str...>::contains_outside_parens(plus_s{}) && !(String<str...>{}.string[0] == ' '), String<str...>>)
{
  return parse_binop<'+', str...>(String<str...>{});
}
//* operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!is_deref<str...>() && String<str...>::contains_outside_parens(times_s{}) && !(String<str...>{}.string[0] == ' '), String<str...>>)
{
  return parse_binop<'*', str...>(String<str...>{});
}
// / operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!is_deref<str...>() && String<str...>::contains_outside_parens(div_s{}) && !(String<str...>{}.string[0] == ' '), String<str...>>)
{
  return parse_binop<'/', str...>(String<str...>{});
}
//- operator case
template <char... str>
constexpr auto _parse_expression(std::enable_if_t<!is_deref<str...>() && String<str...>::contains_outside_parens(minus_s{}) && !(String<str...>{}.string[0] == ' '), String<str...>>)
{
  return parse_binop<'-', str...>(String<str...>{});
}

/*
template<char... str>
constexpr auto _parse_expression(String<str...>){
        return Expression<bool>{};
        }//*/
}

template <char... str>
constexpr auto parse_expression(mutils::String<str...>)
{
  using namespace parse_expressions;
  return parse_phase::Expression<std::decay_t<decltype(_parse_expression<str...>(String<str...>{}))>>{};
}
}
}
