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

template<HandleAccess ha>
using canWrite = std::integral_constant<bool,ha == HandleAccess::write ? true 
		: (ha == HandleAccess::all ? 
		   true : false)>;

template<HandleAccess ha>
using canRead = std::integral_constant<bool,
									   ha == HandleAccess::read ? true
									   : (ha == HandleAccess::all ? 
										  true : false)>;

constexpr bool can_flow(Level from, Level to){
	return to == Level::causal || from == Level::strong;
}
