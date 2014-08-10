#pragma once

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
