#pragma once
#include <string>

enum class Level { causal, strong, undef};

constexpr bool runs_with_strong(Level l){
	return (l == Level::strong) || (l == Level::undef);
}

constexpr bool runs_with_causal(Level l){
	return l == Level::causal;
}

constexpr Level _min_of_levels(){
	return Level::undef;
}

template<typename... Levels>
constexpr Level _min_of_levels(Level l, Levels... a){
	if (l == Level::causal) return l;
	else return _min_of_levels(a...);
}

constexpr Level _max_of_levels(){
	return Level::undef;
}

template<typename... Levels>
constexpr Level _max_of_levels(Level l, Levels... a){
	if (l == Level::strong) return l;
	else return _max_of_levels(a...);
}


template<typename... Levels>
constexpr Level min_of_levels(Levels... l){
	Level l1 = _min_of_levels(l...);
	Level l2 = _max_of_levels(l...);
	if (l1 == Level::undef) return l2;
	else return l1;
}

template<typename... Levels>
constexpr Level max_of_levels(Levels... l){
	Level l1 = _min_of_levels(l...);
	Level l2 = _max_of_levels(l...);
	if (l2 == Level::undef) return l1;
	else return l2;
}

template<Level l>
using level_constant = std::integral_constant<Level,l>;


template<Level l>
std::string levelStr(){
	const static std::string ret =
		(l == Level::strong ? "strong" :
		 (l == Level::causal ? "weak" : "undef"));
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
	return to == Level::causal
		|| to == Level::undef
		|| from == Level::strong
		|| from  == Level::undef;
}
