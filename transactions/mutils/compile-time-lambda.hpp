#pragma once

namespace mutils{

	namespace ct_lambda{

		template<class F>
		struct wrapper
		{
			using ftype = F;
			static_assert(std::is_empty<F>(), "Lambdas must be empty");
			template<class... Ts>
			decltype(auto) operator()(Ts&&... xs) const
				{
					return reinterpret_cast<const F&>(*this)(std::forward<Ts>(xs)...);
				}
		};

		struct wrapper_factor
		{
			template<class F>
			constexpr wrapper<F> operator += (F*)
				{
					return {};
				}
		};

		struct addr_add
		{
			template<class T>
			friend typename std::remove_reference<T>::type *operator+(addr_add, T &&t)
				{
					return &t;
				}
		};

	}
#define STATIC_LAMBDA ct_lambda::wrapper_factor() += true ?	\
		nullptr :											\
		ct_lambda::addr_add() + []

}
