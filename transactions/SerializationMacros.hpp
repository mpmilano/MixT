#pragma once
#define DEFAULT_SERIALIZE2(a,b) int to_bytes(char* ret) const {		\
		int sa = ::to_bytes(a,ret);										\
		return sa + ::to_bytes(b,ret + sa);								\
	}																	\
	int bytes_size() const {											\
		return ::bytes_size(a) + ::bytes_size(b);						\
	}

#define DEFAULT_DESERIALIZE3(Name,a,b)							\
	template<typename T>										\
	static Name from_bytes(char* v, const T& manager){			\
		auto a2 = ::from_bytes<decltype(a)>(v,manager);					\
		Name r{a2,::from_bytes<decltype(b)>(v + ::bytes_size(a2),manager)}; \
		return r;												\
	}

#define DEFAULT_MANAGER2(a,b) 	const std::pair<decay<decltype(a.manager())>, \
												decay<decltype(b.manager())> >&	\
	manager() const {													\
		static std::pair<decay<decltype(a.manager())>,					\
						 decay<decltype(b.manager())> >					\
			r{val.manager(),next.manager()};							\
		return r;														\
	}

	
#define DEFAULT_SERIALIZE_IMPL2(count, ...) DEFAULT_SERIALIZE ## count (__VA_ARGS__)
#define DEFAULT_SERIALIZE_IMPL(count, ...) DEFAULT_SERIALIZE_IMPL2(count, __VA_ARGS__)
#define DEFAULT_SERIALIZE(...) DEFAULT_SERIALIZE_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)


#define DEFAULT_DESERIALIZE_IMPL2(count, ...) DEFAULT_DESERIALIZE ## count (__VA_ARGS__)
#define DEFAULT_DESERIALIZE_IMPL(count, ...) DEFAULT_DESERIALIZE_IMPL2(count, __VA_ARGS__)
#define DEFAULT_DESERIALIZE(...) DEFAULT_DESERIALIZE_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)

#define DEFAULT_MANAGER_IMPL2(count, ...) DEFAULT_MANAGER ## count (__VA_ARGS__)
#define DEFAULT_MANAGER_IMPL(count, ...) DEFAULT_MANAGER_IMPL2(count, __VA_ARGS__)
#define DEFAULT_MANAGER(...) DEFAULT_MANAGER_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)

#define DEFAULT_SERIALIZATION_SUPPORT(a,b...) DEFAULT_MANAGER(b)	\
	DEFAULT_SERIALIZE(b) DEFAULT_DESERIALIZE(a,b)

#define SERIALIZATION_CLASS(a,b) ByteRepresentable<std::pair<a,b> >
