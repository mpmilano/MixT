#pragma once
#include "Transaction.hpp"

namespace myria { namespace mtl {

		template<typename T>
		class Print : public ConStatement<get_level<T>::value> {
		public:

			T t;
	
			Print(const T& t):t(t){
				static_assert(is_ConExpr<T>::value,"Error: print expressions.");
			}
	
			bool operator==(const Print &p) const {return t == p.t;}

			template<Level l>
			bool operator==(const ConStatement<l>& c) const {
				if (Print* n = dynamic_cast<Print*>(&c)) return (*n == *this);
				else return false;
			}

			auto environment_expressions() const {
				return mtl::environment_expressions(t);
			}

			bool strongCall(TransactionContext* ctx, StrongCache& a, const StrongStore& b) const {
				auto ret = mtl::run_ast_strong(ctx,a,b,t);
				if (runs_with_strong(get_level<T>::value)) {
					std::cout << ret << std::endl;
				}
				return true;
			}
			bool causalCall(TransactionContext* ctx, CausalCache& a, const CausalStore& b) const {
				auto ret = mtl::run_ast_causal(ctx,a,b,t);
				if (runs_with_causal(get_level<T>::value)){
					std::cout << ret << std::endl;
				}
				return true;
			}

		};


		template<typename T>
		constexpr Level chld_min_level_f(Print<T> const * const ){
			return get_level<T>::value;
		}


		template<unsigned long long ID, typename l>
		auto find_usage(const Print<l>& p){
			return find_usage<ID>(p.t);
		}

		template<typename T>
		auto print(const T& t){
			return Print<T>{t};
		}

		class Print_Str {
		public:
	
			std::string t;
			bool print_at_strong;
	
			Print_Str(const std::string& t, bool print_at_strong = true)
				:t(t),print_at_strong(print_at_strong){
			}
	
			bool operator==(const Print_Str &p) const {return t == p.t;}

			template<Level l>
			bool operator==(const ConStatement<l>& c) const {
				if (Print_Str* n = dynamic_cast<Print_Str*>(&c)) return (*n == *this);
				else return false;
			}

			auto environment_expressions() const {
				return std::tuple<>();
			}

			bool strongCall(TransactionContext* ctx, StrongCache& a, const StrongStore& b) const {
				if (print_at_strong)
					std::cout << t << std::endl;
				return true;
			}
	
			constexpr bool causalCall(TransactionContext* ctx, CausalCache& a, const CausalStore& b) const {
				if (!print_at_strong)
					std::cout << t << std::endl;
				return true;
			}

		};

		template<Level l>
		struct PS : public ConStatement<l>, public Print_Str
		{
			PS(const Print_Str &t):Print_Str(t.t,runs_with_strong(l)){}
		};

		template<Level l>
		constexpr Level chld_min_level_f(PS<l> const * const ){
			return l;
		}

		template<typename PrevBuilder>
		auto append(const PrevBuilder &pb, const Print_Str &ps){
			return append(pb,PS<PrevBuilder::pc::value>{ps});
		}

		template<unsigned long long ID>
		auto find_usage(const Print_Str& ){
			return nullptr;
		}

		template<typename T>
		auto print_str(const T& t){
			return Print_Str{t};
		}


	} }
