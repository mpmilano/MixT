#pragma once
#include "mtl/builtins_declarations.hpp"
#include "mtl/typecheck_and_label_decl.hpp"
#include "mtl/typecheck_handle_operations.hpp"
#include "mtl/replace_type.hpp"

namespace myria { namespace mtl { 

		
		using true_binding = type_binding<mutils::String<'t','r','u','e'>, bool, Label<top>,type_location::local>;
		using false_binding = type_binding<mutils::String<'f','a','l','s','e'>, bool, Label<top>,type_location::local>;
		using list_binding = type_binding<typename builtins::ListStub::name, typename builtins::ListStub, Label<bottom>, type_location::local >;
		
		namespace parse_expressions{
			template <char...>
			constexpr auto _parse_expression(typename builtins::ListStub::name a)
			{
				using namespace parse_phase;
				return VarReference<DECT(a)>{};
			}
		}

		namespace typecheck_phase {

			MYRIA_SPECIAL_OPERATIONS(typename builtins::ListStub::push_back_name,
															 void,
															 static_assert(std::is_same<handle,builtins::ListStub>::value),
															 typename builtins::ListStub::label
				);

			MYRIA_SPECIAL_OPERATIONS(typename builtins::NulledOp::name,
															 handle,
															 ;,
															 typename handle::label
				);
		}		
	}}
