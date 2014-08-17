#pragma once
#include "subset.hpp"
#include <functional>
#include <type_traits>
#include <memory>

namespace backend {

	template<Client_Id cid>
	class pvt1;

	template<Client_Id cid>
	class Client<cid>::transaction_cls {
	private:
		bool sync = false;
		Client &c;
		transaction_cls(Client& c);
		
		template<Level l>
		void waitForSync();
			
		template<typename... T>
		static constexpr typename std::enable_if<sizeof...(T) == 0, Level>::type 
		meet(Level l1, T...){
			return l1;
		}

		template<typename... T>
		static constexpr typename std::enable_if< (sizeof...(T) > 0), Level>::type 
		meet(Level l1, T... args){
			return (is_causal(l1) ? Level::causal : 
					meet(args...)
				);
		}

		template<typename... T>
		static constexpr typename std::enable_if<sizeof...(T) == 0, Level>::type 
		join(Level l1, T...){
			return l1;
		}

		template<typename... T>
		static constexpr typename std::enable_if< (sizeof...(T) > 0), Level>::type 
		join(Level l1, T... args){
			return (is_strong(l1) ? Level::strong : 
					join(args...)
				);
		}

		
	public:

		class rw_cls;

		template < typename R, typename... Args>
		auto ro(R &f, Args... args);
		
		template < typename R, typename... Args>
		void wo(R &f, Args... args);
		
		template < typename R, typename... Args>
		auto rw(R &f, Args... args);
		
		~transaction_cls();
		
		friend class Client;
		friend class pvt1<cid>;
	};
}
