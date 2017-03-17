#pragma once

#include "top.hpp"

namespace myria { namespace pgsql {

		struct causal{};
		struct strong{};
		
		template<> struct Label<causal>{

			constexpr Label() = default;
			
			constexpr static bool flows_to(const Label<bottom>&){
				return false;
			}

			constexpr static bool flows_to(const Label<top>&){
				return true;
			}

			constexpr static bool flows_to(const Label<strong>&){
				return true;
			}

			constexpr static bool flows_to(const Label&){
				return true;
			}

			template<typename... lbls>
			constexpr static auto min_with(const lbls&...){
				return std::conditional_t<exists<std::is_same<lbls,Label<bottom> >::value...>(),
																	Label<bottom>,
																	Label
																	>{};
			}

			template<typename... lbls>
			constexpr static bool is_min_of(const lbls&...){
				constexpr bool res = !exists<std::is_same<lbls,Label<bottom> >::value...>();
				return res;
			}

			template <typename... labels>
			constexpr static bool is_max_of(const labels&...)
				{
					constexpr bool res = !(exists<std::is_same<lbls,Label<top> >::value...>()
																 || exists<std::is_same<lbls,Label<strong> >::value...>());
					return res;
				}
			
			using is_strong = std::false_type;
			using is_causal = std::true_type;
			
			using requires_causal_tracking = std::true_type;
			static constexpr char description[] = "causal";
		};

		template<> struct Label<strong>{

			constexpr Label() = default;

			constexpr static bool flows_to(const Label<bottom>&){
				return false;
			}

			constexpr static bool flows_to(const Label<top>&){
				return true;
			}

			constexpr static bool flows_to(const Label<causal>&){
				return false;
			}

			constexpr static bool flows_to(const Label&){
				return true;
			}

			template<typename... lbls>
			constexpr static auto min_with(const lbls&...){
				return std::conditional_t<exists<std::is_same<lbls,Label<bottom> >::value...>(),
																	Label<bottom>,
																	std::conditional_t<exists<std::is_same<lbls,Label<causal> >::value...>(),
																										 Label<causal>,
																										 Label > >{};
			}

			template<typename... lbls>
			constexpr static bool is_min_of(const lbls&...){
				constexpr bool res =  !(exists<std::is_same<lbls,Label<bottom> >::value...>()
																|| exists<std::is_same<lbls,Label<causal> >::value...>());
				return res;
			}

			template <typename... labels>
			constexpr static bool is_max_of(const labels&...)
				{
					constexpr bool res = !exists<std::is_same<lbls,Label<top> >::value...>();
					return res;
				}

			using is_strong = std::true_type;
			using is_causal = std::false_type;

			using requires_causal_tracking = std::false_type;

			static constexpr char description[] = "strong";
		};
		char Label<strong>::description[] = "strong";
		char Label<causal>::description[] = "causal";
	}}
