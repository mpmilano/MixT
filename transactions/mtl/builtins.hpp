#pragma once
#include "mtl/builtins_declarations.hpp"
#include "mtl/typecheck_and_label_decl.hpp"
#include "mtl/typecheck_handle_operations.hpp"
#include "mtl/replace_type.hpp"

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

			MYRIA_SPECIAL_OPERATIONS(typename builtins::NulledOp::name,
															 handle,
															 ;,
															 typename handle::label
				);
		}
		namespace builtins {

			namespace tp{
				using namespace typecheck_phase;

				template<typename name, typename type, typename list_name>
				struct discovery_triple;
				
				template<typename discovered, typename AST>
				constexpr auto __do_your_thing(discovered, AST);
				
				template<typename Accum, typename AST>
				using Do_your_thing = DECT(__do_your_thing<Accum, AST>(Accum{},AST{}));
				
				template<typename discovered, typename l, typename l2, typename stub, typename l3, typename lst_var, typename y, typename arg>
				constexpr auto _do_your_thing(discovered, Statement<l,Operation<typename ListStub::push_back_name, Expression<l2, stub, VarReference<lst_var> >, Expression<l3,y,VarReference<arg> > > >){
					return discovered::template add<discovery_triple<arg,y,lst_var> >();
				}
					
					template<typename discovered, typename AST>
					constexpr auto _do_your_thing(discovered, AST){
					return typename AST::template fold<Do_your_thing,discovered>{};
				}

				template<typename discovered, typename AST>
				constexpr auto __do_your_thing(discovered a, AST b){
					return _do_your_thing(a,b);
				}

				template<typename AST>
				constexpr auto do_retyping(mutils::typeset<>, AST a){
					return a;
				}
				
				template<typename name, typename type, typename list_name, typename AST>
				constexpr auto do_retyping(mutils::typeset<discovery_triple<name, type, list_name> >, AST){
					return replace_type<ListStub, List<type>, AST>();
				}
				
			}
			
			template<typename AST>
			constexpr auto do_your_thing(AST a){
				return tp::do_retyping(typename AST::template fold<tp::Do_your_thing, mutils::typeset<> >{}, a);
			}
		}

		namespace runnable_transaction {
			
		}
		
	}}
