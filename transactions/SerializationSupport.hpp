#pragma once
#include "utils.hpp"
#include "SerializationMacros.hpp"
#include "macro_utils.hpp"

template<typename Manager>
struct ByteRepresentable {
	virtual int to_bytes(char* v) const = 0;
	virtual int bytes_size() const = 0;

	//also need to support from_bytes
	//virtual static T from_bytes(char *v, const Manager&)  = 0;
	virtual const Manager& manager() const = 0;
};

template<typename M>
int to_bytes(const ByteRepresentable<M>& b, char* v){
	return b.to_bytes(v);
}

template<typename M>
int bytes_size(const ByteRepresentable<M>& b){
	return b.bytes_size();
}

template<typename T, restrict(std::is_trivially_copyable<T>::value)>
int to_bytes(const T &t, char* v){
	assert(memcpy(v,t,sizeof(T)));
	return sizeof(T);
}

template<typename T, restrict2(std::is_trivially_copyable<T>::value)>
auto bytes_size(const T&){
	return sizeof(T);
}

template<typename T, typename M,
		 restrict(std::is_base_of<ByteRepresentable<M> CMA T>::value)>
auto from_bytes(char *v, const M &m){
	return T::from_bytes(v,m);
}

template<typename T, typename M,
		 restrict2(std::is_trivially_copyable<T>::value)>
auto from_bytes(char *v, const M &){
	T t;
	memcpy(&t,v,sizeof(T));
	return t;
}

template<typename T, typename M>
auto from_bytes_stupid(T* t, char* v, const M& m){
	return from_bytes<T>(v,m);
}
