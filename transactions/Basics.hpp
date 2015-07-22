#pragma once
#include <string>

enum class Level { causal, strong};

constexpr bool is_strong(Level l){
	return l == Level::strong;
}

constexpr bool is_causal(Level l){
	return l == Level::causal;
}


template<Level l>
std::string levelStr(){
	const static std::string ret = (l == Level::strong ? "strong" : "weak");
	return ret;
}

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

