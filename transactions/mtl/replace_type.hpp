#pragma once
#include "mtl/typecheck_and_label_decl.hpp"

namespace myria {namespace mtl { namespace typecheck_phase {
			template<typename original, typename newt>
			struct replace_type_str{
				
				template<typename accum, typename l, typename y, typename e>
				static constexpr auto replace(accum, Expression<l,y,e>, std::enable_if_t<!std::is_same<y,original>::value>* = nullptr);

				template<typename accum, typename l, typename e>
				static constexpr auto replace(accum, Expression<l,original,e>);

				template<typename accum, typename l, typename e>
				static constexpr auto replace(accum, Statement<l,e>);

				template<typename accum, typename l, typename y, typename v, typename e>
				static constexpr auto replace(accum, Binding<l,y,v,e>, std::enable_if_t<!std::is_same<y,original>::value>* = nullptr);

				template<typename accum, typename l, typename v, typename e>
				static constexpr auto replace(accum, Binding<l,original,v,e>);

				template<typename accum, typename ast> using Replace = DECT(replace_type_str::replace(accum{},ast{}));
				
			};

			template<typename original, typename newt>
			template<typename accum, typename l, typename y, typename e>
			constexpr auto replace_type_str<original,newt>::replace(accum, Expression<l,y,e>,
																																		 std::enable_if_t<!std::is_same<y,original>::value>*){
				return typename Expression<l,y,e>::template default_traverse<Replace, accum>{};
			}

			template<typename original, typename newt>
			template<typename accum, typename l, typename e>
			constexpr auto replace_type_str<original,newt>::replace(accum, Expression<l,original,e>){
				return typename Expression<l,newt,e>::template default_traverse<Replace, accum>{};
			}

			template<typename original, typename newt>
			template<typename accum, typename l, typename e>
			constexpr auto replace_type_str<original,newt>::replace(accum, Statement<l,e>){
				return typename Statement<l,e>::template default_traverse<Replace, accum>{};
			}

			template<typename original, typename newt>
			template<typename accum, typename l, typename y, typename v, typename e>
			constexpr auto replace_type_str<original,newt>::replace(accum, Binding<l,y,v,e>, std::enable_if_t<!std::is_same<y,original>::value>*){
				return typename Binding<l,y,v,e>::template default_traverse<Replace, accum>{};
			}

			template<typename original, typename newt>
			template<typename accum, typename l, typename v, typename e>
			constexpr auto replace_type_str<original,newt>::replace(accum, Binding<l,original,v,e>){
				return typename Binding<l,newt,v,e>::template default_traverse<Replace, accum>{};
			}


			template<typename old, typename newt, typename AST>
			constexpr auto replace_type(){
				return replace_type_str<old,newt>::replace(nullptr,AST{});
			}
			
		}}}
