#pragma once

#include "top.hpp"
#include "AST_split.hpp"

namespace myria { namespace pgsql {

		struct causal{};
		struct strong{};
		
	}
		template<> struct Label<pgsql::causal>{

			constexpr Label() = default;
			
			constexpr static bool flows_to(const Label<bottom>&){
				return true;
			}

			constexpr static bool flows_to(const Label<top>&){
				return false;
			}

			constexpr static bool flows_to(const Label<pgsql::strong>&){
				return false;
			}

			constexpr static bool flows_to(const Label&){
				return true;
			}

			template<typename... lbls>
			constexpr static auto min_with(const lbls&...){
				return std::conditional_t<mutils::exists<std::is_same<lbls,Label<bottom> >::value...>(),
																	Label<bottom>,
																	Label
																	>{};
			}

			template<typename... lbls>
			constexpr static bool is_min_of(const lbls&...){
				constexpr bool res2 = (lbls::flows_to(Label{}) && ... && true);
				constexpr bool res = !mutils::exists<std::is_same<lbls,Label<bottom> >::value...>();
				static_assert(res == res2);
				return res;
			}

			template <typename... lbls>
			constexpr static bool is_max_of(const lbls&...)
				{
					constexpr bool res2 = (Label::flows_to(lbls{}) && ... && true);
					constexpr bool res = !(mutils::exists<std::is_same<lbls,Label<top> >::value...>()
																 || mutils::exists<std::is_same<lbls,Label<pgsql::strong> >::value...>());
					static_assert(res == res2);
					return res;
				}
			
			using is_strong = std::false_type;
			using is_causal = std::true_type;
			
			using requires_causal_tracking = std::true_type;
			using is_label = std::true_type;
			
			static constexpr char description[] = "causal";
		};

		template<> struct Label<pgsql::strong>{

			constexpr Label() = default;

			constexpr static bool flows_to(const Label<bottom>&){
				return true;
			}

			constexpr static bool flows_to(const Label<top>&){
				return false;
			}

			constexpr static bool flows_to(const Label<pgsql::causal>&){
				return true;
			}

			constexpr static bool flows_to(const Label&){
				return true;
			}

			template<typename... lbls>
			constexpr static auto min_with(const lbls&...){
				return std::conditional_t<mutils::exists<std::is_same<lbls,Label<bottom> >::value...>(),
																	Label<bottom>,
																	std::conditional_t<mutils::exists<std::is_same<lbls,Label<pgsql::causal> >::value...>(),
																										 Label<pgsql::causal>,
																										 Label > >{};
			}

			template<typename... lbls>
			constexpr static bool is_min_of(const lbls&...){
				constexpr bool res2 = (lbls::flows_to(Label{}) && ... && true);
				constexpr bool res =  !(mutils::exists<std::is_same<lbls,Label<bottom> >::value...>()
																|| mutils::exists<std::is_same<lbls,Label<pgsql::causal> >::value...>());
				static_assert(res == res2);
				return res;
			}

			template <typename... lbls>
			constexpr static bool is_max_of(const lbls&...)
				{
					constexpr bool res2 = (Label::flows_to(lbls{}) && ... && true);
					constexpr bool res = !mutils::exists<std::is_same<lbls,Label<top> >::value...>();
					static_assert(res == res2);
					return res;
				}

			using is_strong = std::true_type;
			using is_causal = std::false_type;

			using requires_causal_tracking = std::false_type;
			using is_label = std::true_type;

			static constexpr char description[] = "strong";
		};

	namespace mtl { namespace split_phase {
			static_assert(!are_equivalent(Label<top>{}, Label<pgsql::strong>{} ));
			static_assert(!are_equivalent(Label<top>{}, Label<pgsql::causal>{} ));
			static_assert(!are_equivalent(Label<top>{}, Label<bottom>{} ));
			static_assert(!are_equivalent(Label<bottom>{}, Label<pgsql::strong>{} ));
			static_assert(!are_equivalent(Label<bottom>{}, Label<pgsql::causal>{} ));
			static_assert(!are_equivalent(Label<pgsql::causal>{}, Label<pgsql::strong>{} ));
		}}

	}
