#pragma once
#include "ConExpr.hpp"

	static int temporary_class_19837491_id_pool = 0;
	
//the level here is for referencing the temporary later.
//it's the endorsement check!
	template<backend::Level l, typename T>
	struct Temporary : public ConStatement<get_level<T>::value> {
		static_assert(is_ConExpr<T>::value,
					  "Error: can only assign temporary the result of expressions");
		static_assert(l == backend::Level::causal ||
					  get_level<T>::value == backend::Level::strong,
					  "Error: flow violation");
	public:
		const int id;
		const T t;
		Temporary(const T& t):id(++temporary_class_19837491_id_pool),t(t){}
		
		auto getReadSet() const {
			return t.getReadSet();
		}
		
		bool operator()(Store &s) const {
			typedef typename std::decay<decltype(t(s))>::type R;
			if (!s.contains(id)) s[id].reset((Store::stored) new R(t(s)));
			return true;
		}
			template<Level l2, typename i2>
			friend std::ostream & operator<<(std::ostream &os, const Temporary<l2,i2>&);
};	

template<backend::Level l, typename T>
auto make_temp(const T& t){
	return Temporary<l,T>(t);
}

template<backend::Level, backend::Level l>
auto make_temp(const DummyConExpr<l>& r){
	return r;
}
template<Level l2, typename i2>
std::ostream & operator<<(std::ostream &os, const Temporary<l2,i2>& t){
	return os << "x" << t.id << "<" << levelStr<l2>() << ">" <<  " = " << t.t;
}

