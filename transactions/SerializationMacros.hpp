#pragma once
#define DEFAULT_SERIALIZE2(a,b) int to_bytes(char* ret) const {		\
		int sa = ::to_bytes(a,ret);										\
		return sa + ::to_bytes(b,ret + sa);								\
	}																	\
	int bytes_size() const {											\
		return ::bytes_size(a) + ::bytes_size(b);						\
	}
#define DEFAULT_SERIALIZE3(a,b,c) int to_bytes(char* ret) const {		\
		int sa = ::to_bytes(a,ret);										\
		int sb = ::to_bytes(b,ret + sa);								\
		return sa + sb + ::to_bytes(c,ret + sa + sb);					\
	}																	\
	int bytes_size() const {											\
		return ::bytes_size(a) + ::bytes_size(b) + ::bytes_size(c);		\
	}

#define DEFAULT_DESERIALIZE3(Name,a,b)			\
	static std::unique_ptr<Name> from_bytes(char* v){				\
		auto a2 = ::from_bytes<std::decay_t<decltype(a)> >(v);			\
		Name r{*a2,*(::from_bytes<std::decay_t<decltype(b)> >(v + ::bytes_size(*a2)))}; \
		return heap_copy(r);						\
	}
#define DEFAULT_DESERIALIZE4(Name,a,b,c)							\
	static std::unique_ptr<Name> from_bytes(char* v){				\
		auto a2 = ::from_bytes<std::decay_t<decltype(a)> >(v);			\
		auto size_a2 = ::bytes_size(*a2);								\
		auto b2 = ::from_bytes<std::decay_t<decltype(b)> >(v + size_a2); \
		Name r{*a2,*b2,*(::from_bytes<std::decay_t<decltype(c)> >(v + size_a2 + ::bytes_size(*b2)))}; \
		return heap_copy(r);						\
	}

#define DEFAULT_SERIALIZE1(a) int to_bytes(char* ret) const {		\
		return ::to_bytes(a,ret);										\
	}																	\
	int bytes_size() const {											\
		return ::bytes_size(a);											\
	}

#define DEFAULT_DESERIALIZE2(Name,a)								\
	static std::unique_ptr<Name> from_bytes(char* v){				\
		auto a2 = ::from_bytes<std::decay_t<decltype(a)> >(v);		\
		Name r{*a2};												\
		return heap_copy(r);						\
	}

	
#define DEFAULT_SERIALIZE_IMPL2(count, ...) DEFAULT_SERIALIZE ## count (__VA_ARGS__)
#define DEFAULT_SERIALIZE_IMPL(count, ...) DEFAULT_SERIALIZE_IMPL2(count, __VA_ARGS__)
#define DEFAULT_SERIALIZE(...) DEFAULT_SERIALIZE_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)


#define DEFAULT_DESERIALIZE_IMPL2(count, ...) DEFAULT_DESERIALIZE ## count (__VA_ARGS__)
#define DEFAULT_DESERIALIZE_IMPL(count, ...) DEFAULT_DESERIALIZE_IMPL2(count, __VA_ARGS__)
#define DEFAULT_DESERIALIZE(...) DEFAULT_DESERIALIZE_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)


#define DEFAULT_SERIALIZATION_SUPPORT(a,b...)		\
	DEFAULT_SERIALIZE(b) DEFAULT_DESERIALIZE(a,b)


