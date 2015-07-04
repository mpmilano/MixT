#pragma once
#include "extras"
#include "BitSet.hpp"

namespace backend {
	//"strong" is Top here.  Linearizable, + start-time ordered
	//"causal" is GLUT.
	enum class Level { causal, strong};
	
	constexpr bool is_strong(Level l){
		return l == Level::strong;
	}

	constexpr bool is_causal(Level l){
		return l == Level::causal;
	}

}

namespace tester{	
	template<backend::Level L, typename R, typename IR>
	class Fuzz;
}

namespace backend{


	enum class HandleAccess {read, write, all};
	
	static constexpr bool canWrite(HandleAccess ha){
		return ha == HandleAccess::write ? true 
			: (ha == HandleAccess::all ? 
			   true : false);
	}

	static constexpr bool canRead(HandleAccess ha){
		return ha == HandleAccess::read ? true
			: (ha == HandleAccess::all ? 
			   true : false);
	}

	typedef int Client_Id;

	template<Client_Id cid>
	class Client;

	class HandlePrime;
	
	//So that you can store lists of these things, if you want.
	
	class GenericHandle;
	
		class HandleAbbrev{
		public:
			
			static constexpr std::true_type* CompatibleWithBitset = nullptr;
			const BitSet<HandleAbbrev>::member_t value;
			typedef decltype(value) itype;
			
			//dear programmer; it's on you to make sure that this is true.
			static constexpr int numbits = sizeof(decltype(value));
			
			operator decltype(value)() const {
				return value;
			}
			HandleAbbrev(decltype(value) v):value(v){}
			
			
			bool operator<(const HandleAbbrev& o) const {
				return value < o.value;
			}
			//idea; we use this for tracking the ReadSet.
		};

	
}


//stupid templates and separate compilation
#include "DataStore.hpp"
#include "HandleBackend.cpp"
#include "Handle.cpp"
#include "DataStore.cpp"

//template magic in here.
#include "handle_utils"

#include "Client.hpp"


