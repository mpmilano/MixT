#pragma once
#include "Temporary.hpp"

namespace myria { namespace mtl {

		template<typename T>
		struct TemporaryMutation : public ConStatement<get_level<T>::value> {
			const std::string name;
			const int store_id;
			const T t;

			TemporaryMutation(const std::string &name, int id, const T& t)
				:name(name),store_id(id),t(t) {}

			auto environment_expressions() const {
				return mtl::environment_expressions(t);
			}
	
			auto strongCall(TransactionContext* ctx, StrongCache& c, StrongStore &s) const {
				choose_strong<get_level<T>::value > choice{nullptr};
				return strongCall(ctx,c,s,choice);
			}

			auto strongCall(TransactionContext* ctx, StrongCache& c, StrongStore &s, std::true_type*) const {
				typedef typename std::decay<decltype(run_ast_strong(ctx,c,s,t))>::type R;
				s.emplace<R>(store_id,run_ast_strong(ctx,c,s,t));
				return true;
			}

			void strongCall(TransactionContext* ctx, StrongCache& c, const StrongStore &s, std::false_type*) const {
				t.strongCall(ctx,c,s);
			}

			auto causalCall(TransactionContext* ctx, CausalCache& c, CausalStore &s) const {
				choose_causal<get_level<T>::value > choice{nullptr};
				return causalCall(ctx,c,s,choice);
			}

			auto causalCall(TransactionContext* ctx, CausalCache& c, CausalStore &s,std::true_type*) const {
				typedef typename std::decay<decltype(run_ast_causal(ctx,c,s,t))>::type R;
				s.emplace<R>(store_id,run_ast_causal(ctx,c,s,t));
				return true;
			}

			auto causalCall(TransactionContext* , CausalCache& , CausalStore &, std::false_type*) const {
				//noop.  We've already executed this instruction.
				return true;
			}
	
		};


		template<typename T>
		struct chld_min_level<TemporaryMutation<T> > : chld_min_level<T> {};
		template<typename T>
		struct chld_min_level<const TemporaryMutation<T> > : chld_min_level<T> {};


		template<typename T>
		struct chld_max_level<TemporaryMutation<T> > : chld_min_level<T> {};
		template<typename T>
		struct chld_max_level<const TemporaryMutation<T> > : chld_min_level<T> {};


		template<unsigned long long ID, typename T>
		struct contains_temporary<ID, TemporaryMutation<T> > : contains_temporary<ID,T> {};


		template<unsigned long long ID, typename T>
		auto find_usage(const TemporaryMutation<T> &t){
			return find_usage<ID>(t.t);
		}

	} }
