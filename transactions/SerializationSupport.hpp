#pragma once
#include "utils.hpp"
#include "type_utils.hpp"
#include "SerializationMacros.hpp"
#include "macro_utils.hpp"
#include <vector>

struct ByteRepresentable {
	virtual int to_bytes(char* v) const = 0;
	virtual int bytes_size() const = 0;
	virtual ~ByteRepresentable(){}
	//needs to exist, but can't declare virtual statics
	//virtual static T* from_bytes(char *v) const  = 0;
};

int to_bytes(const ByteRepresentable& b, char* v);

int bytes_size(const ByteRepresentable& b);

template<typename T, restrict(std::is_trivially_copyable<T>::value)>
int to_bytes(const T &t, char* v){
	assert(memcpy(v,&t,sizeof(T)));
	return sizeof(T);
}

template<typename T, restrict2(std::is_trivially_copyable<T>::value)>
auto bytes_size(const T&){
	return sizeof(T);
}

template<typename T,
		 restrict(std::is_base_of<ByteRepresentable CMA T>::value)>
std::unique_ptr<T> from_bytes(char *v){
	return T::from_bytes(v);
}

template<typename T,
		 restrict2(std::is_trivially_copyable<T>::value)>
std::unique_ptr<T> from_bytes(char *v){
	auto t = std::make_unique<T>();
	if (v) {
		memcpy(t.get(),v,sizeof(T));
		return std::move(t);
	}
	else return nullptr;
}

template<typename T>
std::unique_ptr<T> from_bytes_stupid(T* t, char* v) {
	return from_bytes<T>(v);
}

template<typename T>
int to_bytes(const std::set<T>& s, char* _v){
	((int*)_v)[0] = s.size();
	char *v = _v + sizeof(int);
	for (auto &a : s){
		v += to_bytes(a,v);
	}
	return _v - v;
}

template<typename T>
int bytes_size(const std::set<T>& s){
	int size = sizeof(int);
	for (auto &a : s) {
		size += bytes_size(a);
	}
	return size;
}

template<typename>
struct is_set : std::false_type {};

template<typename T>
struct is_set<std::set<T> > : std::true_type {};

template<typename T>
std::unique_ptr<type_check<is_set,T> > from_bytes(char* _v) {
	int size = ((int*)_v)[0];
	char* v = _v + sizeof(int);
	auto r = std::make_unique<std::set<typename T::key_type> >();
	for (int i = 0; i < size; ++i){
		auto e = from_bytes<typename T::key_type>(v);
		v += bytes_size(*e);
		r->insert(*e);
	}
	return std::move(r);
}
