#pragma once
#include "mtl/builtins_declarations.hpp"
#include "mtl/typecheck_and_label_decl.hpp"
#include "mtl/typecheck_handle_operations.hpp"

namespace myria { namespace mtl { 

		namespace parse_expressions{
			template <char...>
			constexpr auto _parse_expression(typename builtins::ListStub::name a)
			{
				using namespace parse_phase;
				return VarReference<DECT(a)>{};
			}
		}

		namespace typecheck_phase {
			template <int seqnum, int depth, typename Env>
			constexpr auto _typecheck(Env, parse_phase::Expression<parse_phase::VarReference<typename builtins::ListStub::name>>)
			{
				using match = builtins::ListStub;
				using label = typename match::label;
				using type = typename match::type;
				return Expression<label, type, VarReference<typename builtins::ListStub::name>>{};
			}

			MYRIA_SPECIAL_OPERATIONS(typename builtins::ListStub::push_back_name,
															 void,
															 static_assert(std::is_same<handle,builtins::ListStub>::value),
															 typename builtins::ListStub::label
				);
		}
	}}
