#pragma once
#include "utils.hpp"

template<typename Manager>
struct ByteRepresentable {
	virtual int to_bytes(void* v) const = 0;
	virtual int bytes_size() const = 0;

	//also need to support from_bytes
	//virtual static T from_bytes(void *v, const Manager&)  = 0;
	virtual const Manager& manager() const = 0;
};

template<typename M>
int to_bytes(const ByteRepresentable<M>& b, void* v){
	return b.to_bytes(v);
}

template<typename M>
int bytes_size(const ByteRepresentable<M>& b){
	return b.bytes_size();
}

template<typename T, restrict(std::is_trivially_copyable<T>::value)>
int to_bytes(const T &t, void* v){
	assert(memcpy(v,t,sizeof(T)));
	return sizeof(T);
}

template<typename T, restrict2(std::is_trivially_copyable<T>::value)>
auto bytes_size(const T&){
	return sizeof(T);
}

template<typename T, typename M,
		 restrict(std::is_base_of<ByteRepresentable<M> CMA T>::value)>
auto from_bytes(void *v, const M &m){
	return T::from_bytes(v,m);
}

template<typename T, typename M,
		 restrict2(std::is_trivially_copyable<T>::value)>
auto from_bytes(void *v, const M &){
	T t;
	memcpy(&t,v,sizeof(T));
	return t;
}

#define DEFAULT_SERIALIZE2(a,b) void * to_bytes(void* ret) const {		\
		int sa = to_bytes(a,ret);										\
		return sa + to_bytes(b,ret + sa);								\
	}																	\
	int bytes_size() const {											\
		return bytes_size(a) + bytes_size(b);							\
	}

#define DEFAULT_DESERIALIZE3(Name,a,b)							\
	template<typename T>										\
	static Name from_bytes(void* v, const T& manager){			\
		auto a2 = from_bytes<decltype(a)>(v,manager);			\
		Name r{a2,from_bytes<decltype(b)>(v + bytes_size(a2),manager)};	\
		return r;												\
	}
	

	
#define DEFAULT_SERIALIZE_IMPL2(count, ...) DEFAULT_SERIALIZE ## count (__VA_ARGS__)
#define DEFAULT_SERIALIZE_IMPL(count, ...) DEFAULT_SERIALIZE_IMPL2(count, __VA_ARGS__)
#define DEFAULT_SERIALIZE(...) STANDARD_BEGIN(DEFAULT_SERIALIZE_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__))


#define DEFAULT_DESERIALIZE_IMPL2(count, ...) DEFAULT_DESERIALIZE ## count (__VA_ARGS__)
#define DEFAULT_DESERIALIZE_IMPL(count, ...) DEFAULT_DESERIALIZE_IMPL2(count, __VA_ARGS__)
#define DEFAULT_DESERIALIZE(...) STANDARD_BEGIN(DEFAULT_DESERIALIZE_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__))
