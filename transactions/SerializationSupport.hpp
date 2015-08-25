#pragma once
#include "utils.hpp"
#include "SerializationMacros.hpp"
#include "macro_utils.hpp"

struct ByteRepresentable {
	virtual int to_bytes(char* v) const = 0;
	virtual int bytes_size() const = 0;
	virtual ~ByteRepresentable(){}
	//needs to exist, but can't declare virtual statics
	//virtual static T* from_bytes(char *v) const  = 0;
};

int to_bytes(const ByteRepresentable& b, char* v){
	return b.to_bytes(v);
}

int bytes_size(const ByteRepresentable& b){
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

template<typename T,
		 restrict(std::is_base_of<ByteRepresentable CMA T>::value)>
T* from_bytes(char *v){
	return T::from_bytes(v);
}

template<typename T,
		 restrict2(std::is_trivially_copyable<T>::value)>
T* from_bytes(char *v){
	T *t = new T();
	memcpy(t,v,sizeof(T));
	return t;
}

template<typename T>
T* from_bytes_stupid(T* t, char* v) {
	return from_bytes<T>(v);
}
