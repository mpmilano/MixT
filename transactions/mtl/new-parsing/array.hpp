#pragma once
#include <cassert>
#include <utility>

template<typename T, std::size_t size> struct array;

template<typename T, typename array> struct iterator{
		array &arr;
		std::size_t index;
		constexpr iterator(array& arr, std::size_t index)
			:arr(arr),index(index){}

		constexpr iterator operator++(){
			++index;
			return *this;
		}
		constexpr bool operator==(const iterator& o){
			//TODO: this is obviously only fine for range-based for.
			return (index == o.index);
		}

		template<typename A>
		constexpr bool operator!=(const A& a){
			return !operator==(a);
		}
		
		constexpr T& operator*() {
			return arr[index];
		}
};

template<typename T> struct array<T,1>{
	T hd[1];
	using iterator = ::iterator<T,array>;
	constexpr auto size() const {
		return size;
	}
	constexpr T& operator[](std::size_t i){
		assert(i == 0);
		return hd[0];
	}

	constexpr const T& operator[](std::size_t i) const {
		assert(i == 0);
		return hd[0];
	}
	
	constexpr T* ptr(std::size_t i){
		assert(i == 0);
		return hd;
	}

	constexpr const T* ptr(std::size_t i) const {
		assert(i == 0);
		return hd;
	}
	
	constexpr iterator begin(){
		return iterator{*this,0};
	}
	constexpr iterator end() {
		return iterator{*this,1};
	}

	constexpr array(array&& o):hd{{std::move(o.hd[0])}}{}
	constexpr array(){}
};

template<typename T, std::size_t _size> struct array {
	T hd[1];
	array<T,_size-1> rest;

	using iterator = ::iterator<T,array>;

	constexpr auto size() const {
		return size;
	}


	constexpr T* ptr(std::size_t i){
		if (i == 0) return hd;
		else {
			if (i < _size){
				return rest.ptr(i-1);
			}
			else {
				assert(false && "error: index out of bounds");
			}
		}
	}

	constexpr const T* ptr(std::size_t i) const {
		if (i == 0) return hd;
		else {
			if (i < _size){
				return rest.ptr(i-1);
			}
			else {
				assert(false && "error: index out of bounds");
			}
		}
	}

	constexpr T& operator[](std::size_t i){
		return *ptr(i);
	}

	constexpr const T& operator[](std::size_t i) const {
		return *ptr(i);
	}


	constexpr iterator begin(){
		return iterator{*this,0};
	}
	constexpr iterator end() {
		return iterator{*this,_size};
	}

	constexpr array(array&& o)
		:rest(std::move(o.rest)),hd{{std::move(o.hd[0])}}{}
	constexpr array(){}
};
