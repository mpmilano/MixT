#pragma once

#include <vector>
#include <type_traits>
#include <cassert>

template<typename T> struct Union_elem {
	T t;
	bool is_this_elem;
	constexpr Union_elem(T t):t{t},is_this_elem{true}{}
	constexpr Union_elem():t{},is_this_elem{false}{}

	constexpr Union_elem& operator=(const Union_elem& u){
		t = u.t;
		is_this_elem = u.is_this_elem;
		return *this;
	}

	constexpr Union_elem(Union_elem&& u)
		:t{std::move(u.t)},is_this_elem{u.is_this_elem}{}

	template<typename F, typename R>
	constexpr void map(F &&f, R& r){
		if (is_this_elem) r = f(t);
	}

	template<typename F, typename Accum>
	constexpr Accum* fold(F &&f, Accum *a){
		if (is_this_elem) return f(t,a);
		else return a;
	}
	
};

template<> struct Union_elem<std::nullptr_t> {
	std::nullptr_t t;
	bool is_this_elem;
	constexpr Union_elem(std::nullptr_t t):t{t},is_this_elem{true}{}
	constexpr Union_elem():t{nullptr},is_this_elem{false}{}

	constexpr Union_elem& operator=(const Union_elem& u){
		is_this_elem = u.is_this_elem;
		return *this;
	}

	template<typename F, typename R>
	constexpr void map(F &&f, R& r){
		if (is_this_elem) r = f(t);
	}

	template<typename F, typename R, typename Accum>
	constexpr Accum* fold(F &&f, Accum *ac){
		if (is_this_elem) return f(t,ac);
		else return ac;
	}

	
};

template<typename T1, typename... T> struct Union :
	public Union_elem<T1>, public Union_elem<T>...{

	bool is_initialized{false};

	constexpr bool well_formed() const {
		return ((Union_elem<T1>::is_this_elem + 
		... + Union_elem<T>::is_this_elem) == 1) && is_initialized;
	}
	
	template<typename U>
		constexpr Union(U u, std::enable_if_t<((std::is_same<U,T>::value + ... + 0) > 0)>* = nullptr):
		is_initialized{true},
		Union_elem<U>(u){
			assert(well_formed());
		}

		constexpr Union(T1 u):
		is_initialized{true},
		Union_elem<T1>(u){
			assert(well_formed());
		}

	constexpr Union():
		is_initialized{false}{}
	
	template<typename F>
	constexpr auto map(F&& f){
		using R = std::decay_t<decltype(f(*(T1*)nullptr))>;
		R out_param;
		Union_elem<T1>::map(std::forward<F>(f),out_param);
		(Union_elem<T>::map(std::forward<F>(f),out_param),...);
		return out_param;
	}

	template<typename F, typename Accum>
		constexpr Accum* _fold(F&& , Accum* acc){
		return acc;
	}
	

	template<typename F, typename Accum, typename Tf, typename...Ts>
		constexpr Accum* _fold(F&& f, Accum* acc){
		return _fold<F,Accum,Ts...>
			(std::forward<F>(f),
			 Tf::fold(
				 std::forward<F>(f),acc));
	}
	
	template<typename F, typename Accum>
		constexpr Accum* fold(F&& f, Accum* acc){

		return _fold<F,Accum,Union_elem<T>...>
			(std::forward<F>(f),
			 Union_elem<T1>::fold(
				 std::forward<F>(f),acc));
	}

	constexpr Union& operator=(const Union &u){
		Union_elem<T1>::operator=(u);
		(Union_elem<T>::operator=(u),...);
		return *this;
	}

	constexpr Union(Union&& u)
		:Union_elem<T1>{std::move(u)},
		Union_elem<T>{std::move(u)}...{}

	template<typename U>
	constexpr Union& operator=(const U &u){
		Union_elem<U>* _this = this;
		Union_elem<T1>::is_this_elem = false;
		((Union_elem<T>::is_this_elem = false),...);
		_this->t = u;
		_this->is_this_elem = true;
		is_initialized = true;
		return *this;
	}
	
	template<typename U>
	constexpr Union& operator=(U &&u){
		Union_elem<U>* _this = this;
		Union_elem<T1>::is_this_elem = false;
		((Union_elem<T>::is_this_elem = false),...);
		_this->t = std::move(u);
		_this->is_this_elem = true;
		is_initialized = true;
		return *this;
	}


	template<typename target>
		constexpr Union_elem<target>& get_(){
		return *this;
	}

	template<typename target>
		constexpr const Union_elem<target>& get_() const {
		return *this;
	}

	
	template<typename target>
		constexpr auto& get(){
		assert(get_<target>().is_this_elem);
		return get_<target>().t;
	}

template<typename target>
		constexpr auto& get() const {
		assert(get_<target>().is_this_elem);
		return get_<target>().t;
	}
	
	template<std::size_t target, typename cand1, typename... candn>
		static constexpr auto& _get(Union_elem<cand1> &c1, Union_elem<candn>&... c2){
		if constexpr (target == 0){
				return c1.t;
			}
		else return _get<target-1>(c2...);
	}

	template<std::size_t target, typename cand1, typename... candn>
		static const constexpr auto& _get(const Union_elem<cand1> &c1,
																		 const Union_elem<candn>&... c2){
		if constexpr (target == 0){
				return c1.t;
			}
		else return _get<target-1>(c2...);
	}


	template<std::size_t target>
		constexpr auto& get(){
		return _get<target>(*(Union_elem<T1>*)this, *(Union_elem<T>*)this... );
	}

	template<std::size_t target>
		constexpr const auto& get() const {
		return _get<target>(*(Union_elem<T1>*)this, *(Union_elem<T>*)this... );
	}
	
	static constexpr std::size_t _which(){
		return 0;
	}
	
	template<typename cand1, typename... candn>
	static constexpr std::size_t _which(const Union_elem<cand1> &c1,
																		 const Union_elem<candn>&... c2) {
		if (c1.is_this_elem) return 0;
		else return 1 + Union::_which(c2...);
	}

	constexpr std::size_t which() const {
		auto ret = _which(*(Union_elem<T1>*)this, *(Union_elem<T>*)this... );
		return ret;
	}
	
};

