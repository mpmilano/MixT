#pragma once
#include "Transaction.hpp"

template<typename T>
class Print : public ConStatement<get_level<T>::value> {
public:

	T t;
	
	Print(const T& t):t(t){
		static_assert(is_ConExpr<T>::value,"Error: print expressions.");
	}
	
	bool operator==(const Print &p) const {return t == p.t;}

	template<Level l>
	bool operator==(const ConStatement<l>& c) const {
		if (Print* n = dynamic_cast<Print*>(&c)) return (*n == *this);
		else return false;
	}

	auto handles() const {
		return ::handles(t);
	}

	bool strongCall(Store& a, const Store& b) const {
		auto ret = ::run_ast_strong(a,b,t);
		if (get_level<T>::value == Level::strong){
			std::cout << ret << std::endl;
		}
		return true;
	}
	bool causalCall(Store& a, const Store& b) const {
		auto ret = ::run_ast_causal(a,b,t);
		if (get_level<T>::value == Level::causal){
			std::cout << ret << std::endl;
		}
		return true;
	}

};

template<unsigned long long ID, typename l>
auto find_usage(const Print<l>& p){
	return find_usage<ID>(p.t);
}

template<typename T>
auto print(const T& t){
	return Print<T>{t};
}

class Print_Str : public ConStatement<Level::strong> {
public:

	std::string t;
	
	Print_Str(const std::string& t):t(t){
	}
	
	bool operator==(const Print_Str &p) const {return t == p.t;}

	template<Level l>
	bool operator==(const ConStatement<l>& c) const {
		if (Print_Str* n = dynamic_cast<Print_Str*>(&c)) return (*n == *this);
		else return false;
	}

	auto handles() const {
		return std::tuple<>();
	}

	bool strongCall(Store& a, const Store& b) const {
		std::cout << t << std::endl;
		return true;
	}
	
	constexpr bool causalCall(Store& a, const Store& b) const {
		return true;
	}

};

template<unsigned long long ID>
auto find_usage(const Print_Str& ){
	return nullptr;
}

auto print_str(const std::string& t){
	return Print_Str{t};
}
