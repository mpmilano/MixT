#pragma once

template<typename T>
class Massert : public ConStatement<get_level<T>::value> {
public:

	T t;
	
	Massert(const T& t):t(t){
		static_assert(is_ConExpr<T>::value,"Error: massert expressions.");
	}
	
	bool operator==(const Massert &p) const {return t == p.t;}

	template<Level l>
	bool operator==(const ConStatement<l>& c) const {
		if (Massert* n = dynamic_cast<Massert*>(&c)) return (*n == *this);
		else return false;
	}

	auto handles() const {
		return ::handles(t);
	}

	bool strongCall(Store& a, const Store& b) const {
		auto ret = ::run_ast_strong(a,b,t);
		if (runs_with_strong(get_level<T>::value)){
			assert(ret);
		}
		return true;
	}
	bool causalCall(Store& a, const Store& b) const {
		auto ret = ::run_ast_causal(a,b,t);
		if (runs_with_causal(get_level<T>::value)){
			assert(ret);
		}
		return true;
	}

};

template<unsigned long long ID, typename l>
auto find_usage(const Massert<l>& p){
	return find_usage<ID>(p.t);
}

template<typename T>
auto massert(const T& t){
	return Massert<T>{t};
}
