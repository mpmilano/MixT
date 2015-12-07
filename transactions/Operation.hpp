#pragma once

#include "Handle.hpp"
#include "tuple_extras.hpp"
#include "utils.hpp"
#include "args-finder.hpp"
#include "ConStatement.hpp"
#include "filter-varargs.hpp"
#include "type_utils.hpp"
#include "ConExpr.hpp"
#include "Operation_macros.hpp"
#include "Ends.hpp"

namespace myria { 

	//because we don't have a lot of inspection abilities into this function,
	//we only allow write-enabled handles into which *all* read-enabled handles are
	//permitted to flow.


	//Idea: you can have template<...> above this and it will work!

#define OPERATION(Name, args...) static auto Name ## _impl (args) {	\
		struct r { static bool f(args)

#define END_OPERATION };												\
		auto fp = r::f;													\
		return Operation<extract_store<decltype(fp)>,decltype(fp)>(fp);	\
}

			template<typename, typename>
			struct Operation;

	template<typename T>
	struct Preserve{
		const T t;
	};

	template<typename C, typename T>
	struct Preserve2 {
		const C c;
		const T t;
	};

	template<typename T>
	struct is_preserve : std::false_type {};

	template<typename T>
	struct is_preserve<Preserve<T> > : std::true_type {};

	template<typename C, typename T>
	struct is_preserve<Preserve2<C,T> > : std::true_type {};

	DecayTraits(is_preserve)

	template<typename T>
	auto preserve(const T &t){
		return Preserve<T>{t};
	}

	template<typename C, typename T>
	auto cached(const C &c, const Preserve<T> &t){
		return Preserve2<const C&,T>{c,t.t};
	}

	template<typename T>
	constexpr Level get_level_dref(Preserve<T> const * const){
		//TODO: should the semantics of "preserve" be such that
		//we should recur here? 
		return mtl::get_level<T>::value;
	}

	template<typename T>
	auto handles_helper_2(const Preserve<T>& t){
		return mtl::handles(t.t);
	}

	template<typename C, typename T>
	auto extract_robj_p(const Preserve2<C,T> &t){
		return cached(t.c,t.t);
	}

	template<typename T, restrict(!is_handle<std::decay_t<T> >::value && !is_preserve<T>::value )>
	auto extract_robj_p(T&& t) {
		return std::forward<T>(t);
	}

	template<HandleAccess ha, Level l, typename T>
	auto extract_robj_p(const Handle<l,ha,T>& t)  {
		return &t.remote_object();
	}

	template<typename>
	struct extract_store_str;

	template<typename Ret, typename... Args>
	struct extract_store_str<Ret (*) (Args...)> {
		typedef mutils::extract_match<is_RemoteObj_ptr,Args...> match;
		using type = typename std::remove_pointer<std::decay_t<match> >::type::Store;
	};

	template<typename T>
	using extract_store = typename extract_store_str<T>::type;

	template<typename Store, typename Ret, typename... A>
	struct Operation<Store, Ret (*) (A...)> {
		const bool built_well = false;
		typedef Ret (*F) (A...);
		typedef Ret result_t;
		static constexpr int arity = sizeof...(A);
		typedef std::tuple<A...> args_tuple;
	
		Ret (*fun) (A...);
	
		Operation(Ret (*fun) (A...)):built_well(true),fun(fun) {
		}
	
		Operation():fun([](A...) -> Ret{assert(false && "Operation built on sand!");}) {
		}

		Operation(const Operation<Store, Ret(*) (A...) >& op):built_well(op.built_well),fun(op.fun){
		}

		template<typename ___T, typename Acc>
		using type_check = std::pair<mutils::Rest<mutils::Left<Acc> >,
									 std::integral_constant
									 <bool,
									  sassert2(___T, mutils::First<mutils::Left<Acc> >, 
											   (std::is_same<std::decay_t<___T>, std::decay_t<mutils::First<mutils::Left<Acc> > > >::value || 
												(is_handle<std::decay_t<___T> >::value &&
												 is_RemoteObj_ptr<mutils::First<mutils::Left<Acc> > >::value)
												|| (is_preserve<___T>::value && is_handle<std::decay_t<mutils::First<mutils::Left<Acc> > > >::value) )) && 
									  mutils::Right<Acc>::value
									  > > ;
	
		template<typename... Args>
		auto operator()(Args && ... args) const {
			assert(this);
			assert(built_well && "Calling operation constructed with default constructor is evil!");
			static_assert(sizeof...(Args) == arity, "Error: arity violation");
			typedef mutils::fold_types<type_check,std::tuple<Args...>,
									   std::pair<args_tuple, std::true_type> >
				ft_res;
			static_assert(mutils::Right<ft_res>::value,
						  "Error: TypeError calling operation!");
			constexpr Level min = mtl::min_level<typename
												 mutils::filter<is_readable_handle,Args...>::type
												 >::value;
			constexpr Level max = mtl::max_level<typename
												 mutils::filter<is_writeable_handle,Args...>::type
												 >::value;
			static_assert(can_flow(min,max),"Error: potential flow violation!");
			assert(can_flow(min,max));
		
			//do this here so we abort before causal tracking happens
			discard(Store::tryCast(extract_robj_p(args))...);			

			auto h_read = mutils::filter_tpl<is_readable_handle>(std::make_tuple(args...));
			auto h_strong_read = mutils::filter_tpl<is_strong_handle>(h_read);
			auto h_causal_read = mutils::filter_tpl<is_causal_handle>(h_read);
			auto h_write = mutils::filter_tpl<is_writeable_handle>(std::make_tuple(args...));
			auto h_strong_write = mutils::filter_tpl<is_strong_handle>(h_write);
			auto h_causal_write = mutils::filter_tpl<is_causal_handle>(h_write);
			mutils::foreach(h_strong_read,
							[](const auto &h){h.tracker.afterRead(h.store(),h.name());});
			mutils::foreach(h_causal_read,
							[](const auto &h){h.tracker.waitForRead(h.store(),h.name(),h.remote_object().timestamp());});
			//optimization: track timestamps for causal, only check if they've changed.
			auto causal_pair = mutils::fold(h_causal_read,[](const auto &a, const auto &acc){
					return mutils::tuple_cons(std::make_pair(a.remote_object().timestamp(),a),acc);},std::tuple<>{});
			auto &&ret = fun(Store::tryCast(extract_robj_p(args))...);
			mutils::foreach(causal_pair,
							[](const auto &p){
								if (tracker::ends::is_same(p.first, p.second.remote_object().timestamp())) return;
								else p.second.tracker.afterRead(p.second.store(),p.second.name(),p.second.remote_object().timestamp(),p.second.remote_object().bytes());});
			mutils::foreach(h_strong_write, [](const auto &h){h.tracker.onWrite(h.store(),h.name());});
			mutils::foreach(h_causal_write, [](const auto &h){h.tracker.onWrite(h.store(),h.name(),h.remote_object().timestamp());});
			return ret;
		}
	};

	struct OperationNotFoundError {};

	struct NoOperation {
		static constexpr bool built_well = false;

		typedef NoOperation F;

		template<typename... Args>
		bool operator()(Args && ...) const {
			throw OperationNotFoundError ();
			return false;
		}
	};



#define op_arg(x...) extract_robj_p(x)

} 
