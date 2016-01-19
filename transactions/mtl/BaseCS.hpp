#pragma once
#include "ConStatement.hpp"

namespace myria { namespace mtl {

		template<typename>
		struct chld_min_level;

		template<typename T>
		constexpr bool is_base_CS_f(const T* ){
			return false;
		}
		
		template<Level l>
		class Noop : public ConStatement<l> {
		public:
			Noop(){}
			bool operator==(const Noop &) const {return true;}
			bool operator==(const ConStatement<l>& c) const {
				if (Noop* n = dynamic_cast<Noop>(&c)) return true;
				else return false;
			}

			std::tuple<> handles() const {
				return std::tuple<>();
			}

			constexpr bool strongCall(TransactionContext* ctx, const StrongCache&, const StrongStore&) const {
				return true;
			}
			constexpr bool causalCall(TransactionContext* ctx, const CausalCache&, const CausalStore&) const {
				return true;
			}

			template<Level l2>
			friend std::ostream & operator<<(std::ostream &os, const Noop<l2>&);
		};

		template<Level l>
		struct chld_min_level<Noop<l> > : level_constant<l> {};

		template<unsigned long long ID, Level l>
		auto find_usage(const Noop<l>&){
			return nullptr;
		}

		const Noop<Level::strong> dummy1;
		const Noop<Level::causal> dummy2;


		template<typename T>
		struct is_Noop : std::integral_constant<
			bool,
			std::is_same<T,Noop<Level::strong> >::value ||
			std::is_same<T,Noop<Level::causal> >::value>::type {};

		template<Level l>
		constexpr bool is_base_CS_f(const Noop<l>* ){
			return true;
		}

		//ignore expression

		template<typename T>
		class Ignore : public ConStatement<get_level<T>::value> {
		public:
			T t;
			Ignore(T t):t(t){}
			bool operator==(const Ignore &i) const {return t == i.t;}
			bool operator==(const GCS& c) const {
				if (Ignore* n = dynamic_cast<Ignore>(&c)) return n->t == t;
				else return false;
			}

			auto handles() const {
				return mtl::handles(t);
			}

			bool strongCall(TransactionContext* ctx, StrongCache& c, const StrongStore& s) const {
				run_ast_strong(ctx,c,s,t);
				return true;
			}
			bool causalCall(TransactionContext* ctx, CausalCache& c, const CausalStore& s) const {
				run_ast_causal(ctx,c,s,t);
				return true;
			}

			template<typename T2>
			friend std::ostream & operator<<(std::ostream &os, const Ignore<T2>&);
		};

		template<typename l>
		struct chld_min_level<Ignore<l> > : chld_min_level<l> {};

		template<unsigned long long ID, typename T>
		auto find_usage(const Ignore<T>& i){
			return find_usage(i.t);
		}

		template<typename T>
		struct is_Ignore : std::false_type {};

		template<typename T>
		struct is_Ignore<Ignore<T> > : std::true_type {};

		template<typename T>
		constexpr bool is_base_CS_f(const Ignore<T>* ){
			return true;
		}

		template<typename T>
		Ignore<T> mtl_ignore(T t){
			return Ignore<T>{t};
		}
		

#define CONNECTOR_OP
		
		template<typename T>
		struct is_base_CS : std::integral_constant<bool, is_base_CS_f((T*)nullptr)> {};

	} }
