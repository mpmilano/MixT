#pragma once
#include "ConStatement.hpp"

namespace myria { namespace mtl {

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

			constexpr bool strongCall(const StrongCache&, const StrongStore&) const {
				return true;
			}
			constexpr bool causalCall(const CausalCache&, const CausalStore&) const {
				return true;
			}

			template<Level l2>
			friend std::ostream & operator<<(std::ostream &os, const Noop<l2>&);
		};

		template<typename>
		struct chld_min_level;

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

		template<typename T>
		constexpr bool is_base_CS_f(const T* ){
			return false;
		}

		template<typename T>
		struct is_base_CS : std::integral_constant<bool, is_base_CS_f((T*)nullptr)> {};

#define CONNECTOR_OP 

	} }
