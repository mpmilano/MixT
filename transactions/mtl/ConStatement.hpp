#pragma once

#include "utils.hpp"
#include "tuple_extras.hpp"
#include <string>
#include <memory>
#include <tuple>
#include <iostream>
#include "../BitSet.hpp"
#include "Handle.hpp"
#include "Store.hpp"
#include "Operations.hpp"

namespace myria { namespace mtl {

		template<typename>
		struct is_ConExpr;

		template<typename>
		struct is_ConStatement;

		template<typename T>
		using is_AST_Expr = typename std::integral_constant<
			bool,
			is_ConExpr<T>::value &&
			!std::is_scalar<T>::value &&
			!is_handle<T>::value >::type;


		template<Level l, HandleAccess ha, typename T,typename... Ops>
		auto environment_expressions(const Handle<l,ha,T,Ops...>& h) {
			return std::tuple<>{};
		}

		template<typename T>
		std::enable_if_t<std::is_scalar<T>::value, std::tuple<> > environment_expressions(const T&){
			return std::tuple<>();
		}


		template<typename T, restrict(is_AST_Expr<T>::value || is_ConStatement<T>::value)>
		auto environment_expressions(const T &e){
			return e.environment_expressions();
		}

		template<typename... T>
		auto environment_expressions(const std::tuple<T...> &params){
			return mutils::fold(params,
						[](const auto &e, const auto &acc){
							return std::tuple_cat(mtl::environment_expressions(e),acc);
						}
						,std::tuple<>());
		}

		
		struct GCS {};

		template<Level l>
		struct ConStatement : public GCS{
			static constexpr Level level = l;
		};

		template<typename Arg, typename Acc>
		using contains_temporary_aggr = 
			std::pair<mutils::Left<Acc>, std::integral_constant<bool, (contains_temporary<mutils::Left<Acc>::value, Arg>::value || mutils::Right<Acc>::value) > >;

		template<unsigned long long key, typename Lst>
		using contains_temp_fold = 
			mutils::Right<mutils::fold_types<contains_temporary_aggr, Lst,
											 std::pair<std::integral_constant<unsigned long long, key>,
													   std::true_type> > >;

		template<typename Cls>
		struct is_ConStatement : 
		std::integral_constant<bool, std::is_base_of<GCS,Cls>::value>::type {};


		template<Level l>
		constexpr Level get_level_f(const ConStatement<l>*){
			return l;
		}

		template<Level l>
		constexpr Level get_level_f(const std::integral_constant<Level, l>*){
			return l;
		}

		template< Level l, HandleAccess ha, typename T,typename... Ops>
		constexpr Level get_level_f(const Handle<l,ha,T,Ops...>*){
			return l;
		}


		template<typename A>
		constexpr bool is_tuple_f(A*){
			return false;
		}

		template<typename... Args>
		constexpr bool is_tuple_f(std::tuple<Args...>*){
			return forall(is_ConStatement<Args>::value...);
		}

		template<typename F>
		struct is_cs_tuple : std::integral_constant<bool,
													is_tuple_f((F*) nullptr)
														>::type {};


		template<typename... CS>
		auto call_all_causal(mtl::TransactionContext *ctx, CausalCache& cache, CausalStore &st, const std::tuple<CS...> &t){
			static_assert(mutils::forall_types<is_ConStatement, std::tuple<CS...> >::value, "Error: non-statement found in body");
			bool check = mutils::fold(t,[&cache,&st,ctx](const auto &e, bool b)
									  {assert(! is_ConExpr<std::decay_t<decltype(e)> >::value);
									   return b && e.causalCall(ctx,cache,st);},true);
			assert(check);
			return check;
		}


		template<typename... CS>
		auto call_all_strong(TransactionContext *ctx, StrongCache& cache, StrongStore &st, const std::tuple<CS...> &t){
			static_assert(mutils::forall_types<is_ConStatement, std::tuple<CS...> >::value, "Error: non-statement found in body");
			//TODO: better error propogation please.
			bool check = mutils::fold(t,[&cache,&st,ctx](const auto &e, bool b)
									  {e.strongCall(ctx,cache,st); return true;},true);
			assert(check);
			return check;
		}

		template<typename... T>
		auto stmt_environment_expressions(const std::tuple<T...> &cs){
			return mutils::fold(cs,[](const auto &e, const auto &acc){
					return std::tuple_cat(e.environment_expressions(),acc);
				},
				std::tuple<>());
		}

		struct Base_Builder {};

		template<typename T>
		constexpr bool is_builder(){
			return std::is_base_of<Base_Builder,T>::value;
		}

	} }
