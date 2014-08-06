#pragma once
#include "extras"

namespace backend {
	//"strong" is Top here.  Linearizable, + start-time ordered
	//"causal" is GLUT.
	enum class Level { causal, strong};
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

}


//stupid templates and separate compilation
#include "DataStore.hpp"
#include "HandleBackend.cpp"
#include "Handle.cpp"
#include "DataStore.cpp"

//template magic in here.
#include "handle_utils"

#include "Client.hpp"


