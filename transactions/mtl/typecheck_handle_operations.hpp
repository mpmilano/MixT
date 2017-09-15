#pragma once
#include "typecheck_and_label_decl.hpp"

namespace myria {
namespace mtl {
namespace typecheck_phase {

template <int seqnum, int depth, typename oper_name, typename ptr_label, typename arguments_label_min, typename next_binding_expr, typename ret_t, typename... var_args>
struct handle_operations{
	//void-returning or result unused
	template<typename label_env, typename... Env>
	constexpr static auto handle_operation(std::true_type* /*won't use result*/, type_environment<label_env, Env...> a){
		using old_env = DECT(a);
		using oper_label = resolved_label_min<ptr_label,resolved_label_min<label_env, arguments_label_min> >;
		return Statement<oper_label, Operation<oper_name,next_binding_expr, DECT(typecheck<seqnum,depth+1>(old_env{},var_args{}))...> >{};
	}

	//non-void-returning
	template<typename label_env, typename... Env>
	constexpr static auto handle_operation(std::false_type* /*might use result*/, type_environment<label_env, Env...> a){
		using old_env = DECT(a);
		using operation_execution_label = resolved_label_min<ptr_label,resolved_label_min<label_env, arguments_label_min> >;
		return Expression<operation_execution_label, ret_t, Operation<oper_name,next_binding_expr, DECT(typecheck<seqnum,depth+1>(old_env{},var_args{}))...>>{};
	}
	
};
	}}}
