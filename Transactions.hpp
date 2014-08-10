#pragma once
#include "subset.hpp"
#include <type_traits>

namespace backend {

	template<Client_Id cid>
	class Client<cid>::transaction_cls {
	private:
		bool sync = false;
		Client &c;
		transaction_cls(Client& c);
		
		template<Level l>
		void waitForSync();
		
		template<Level Lmatch, typename T>
		static constexpr auto matches(DataStore::Handle<cid, Lmatch, HandleAccess::read, T> *);
		
		template<Level, typename C>
		static constexpr auto matches(C *); 
		
		template<Level l, typename... Args>
		static constexpr bool matches_any(); 	
		
	public:
		
		typedef int Tid;

		template<Level L, typename T, Tid... depends>
		class ReadRes{
		public:
			template<typename F, Tid... deps2>
			//we can also change the level here.
			ReadRes<L,T,depends..., deps2...> a(F f, ReadRes<T,deps2...> gnu);
		};
		
		template<Level L, Tid... allowed>
		class TransWrite{
		public:
			template<Level L_, typename F, typename T, Tid... depends>
			typename std::enable_if<subset<Tid>::f(subset<Tid>::pack<depends...>(), subset<Tid>::pack<allowed...>()) >::type
			//note - we can enforce a predicate on level here!
			//We're not going to though.
			write(F f, ReadRes<L_,T,depends...>);

		};

		template<Level L, typename T, Tid from>
		class TransRead{
		public:
			operator ReadRes<L,T,from> ();
		};

		template < typename R, typename... Args>
		auto ro(R &f, Args... args);
		
		template < typename R, typename... Args>
		void wo(R &f, Args... args);
		
		template < typename R, typename... Args>
		auto rw(R &f, Args... args);
		
		~transaction_cls();
		
		friend class Client;
	};
}
