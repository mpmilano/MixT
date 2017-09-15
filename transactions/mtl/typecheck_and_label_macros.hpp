#pragma once

#define MYRIA_TYPECHECK_OPERATIONS(oper_name_binding, oper_name_use, rett_def, enableif, type_check, hndl_label) \
	template <int seqnum, int depth, typename old_env, typename choice, oper_name_binding, typename Hndl, typename... var_args> \
	constexpr auto _typecheck(old_env, parse_phase::Operation<oper_name_use, Hndl, parse_phase::operation_args_exprs<>,parse_phase::operation_args_varrefs<var_args...> >, enableif * = nullptr) \
	{																																			\
		using oper_name = oper_name_use;																		\
		using binding_expr = DECT(typecheck<seqnum, depth + 1>(old_env{}, Hndl{}));	\
		using ptr_label = typename binding_expr::label;											\
		using handle = typename binding_expr::yield;												\
		/* we dereference the pointer, which is an influencing action.  Reduce the label
			 of the environment if needed. */																	\
  using arguments_label_min = resolved_label_min_vararg<hndl_label, /*the duplication is on purpose*/ hndl_label, typename DECT(typecheck<seqnum,depth+1>(old_env{},var_args{}))::label...>; \
rett_def;																																\
type_check;																															\
  return handle_operations<seqnum,depth,oper_name,ptr_label,arguments_label_min,binding_expr,ret_t, var_args...>::handle_operation(choice{},old_env{});	\
																																				\
}
	
#define MYRIA_SPECIAL_OPERATIONS(oper_name, rett_t, type_check, hndl_label) MYRIA_TYPECHECK_OPERATIONS(typename, oper_name, using ret_t = rett_t, void*, type_check, hndl_label)
