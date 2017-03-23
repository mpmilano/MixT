#pragma once
#include "postgresql/libpq-fe.h"
#include "Bytes.hpp"
#include "argswrapper.hpp"

//WOW are Oids literally the worst.

namespace myria { namespace pgsql { namespace local{

template<typename >
struct PGSQLinfo;

template<std::size_t indx, typename T>
struct PGSQLinfo_wrapped : public PGSQLinfo<T> {
	template<typename... T2>
	PGSQLinfo_wrapped(T2&& ... t)
		:PGSQLinfo<T>(std::forward<T2>(t)...){}
};
			
template<>
struct PGSQLinfo<long int> {
	static_assert(sizeof(long int) >= 8,"Wow postgres is irritating");
	/*bigint, but actually int8*/
	static constexpr Oid value = 20;
	const long int bigendian;
	char const * const pg_data{(const char*) &bigendian};
	static constexpr std::size_t pg_size{sizeof(long int)};

	
	PGSQLinfo(const long int& li)
		:bigendian(htobe64(li)){}
	
	PGSQLinfo(PGSQLinfo&& o)
		:bigendian(o.bigendian){}
	
	PGSQLinfo(const PGSQLinfo& o)
		:bigendian(o.bigendian){}
	
	static long int from_pg(char const * const v){
		return be64toh(*((long int*)v));
	}
};

template<>
struct PGSQLinfo<bool> {
	static_assert(sizeof(bool)== 1,"Wow postgres is irritating");
	static constexpr Oid value = 16;

	const bool b;
	char const * const pg_data{(char*)&b};
	static constexpr std::size_t pg_size{sizeof(bool)};
	PGSQLinfo(const bool& b):b{b}{}
	
	static bool from_pg(char const * const v){
		return *v != 0;
	}
};


//we're gonna call this a bytea
template<>
struct PGSQLinfo<mutils::Bytes> {
	/*bytea*/
	static constexpr Oid value = 17;
	const std::size_t pg_size;
	std::vector<char> data;
	char const * const pg_data;
	PGSQLinfo(const mutils::Bytes& li)
		:pg_size(li.size),
		 data(li.size,0),
		 pg_data(data.data())
		{
			memcpy(data.data(), li.bytes, li.size);
		}

	static mutils::Bytes from_pg(std::size_t size, char const * const v){
		return mutils::Bytes{v,size};
	}
};


template<typename> struct one { static constexpr int value = 1; };

template<typename T>
struct PGSQLinfo<T&> : public PGSQLinfo<T> {};

template<typename T>
struct PGSQLinfo<const T&> : public PGSQLinfo<T> {};

template<typename T>
struct PGSQLinfo<const T> : public PGSQLinfo<T> {};

			template<typename T> struct dummy_client{};
			template<typename Arg> using PGwrapped = PGSQLinfo_wrapped<Arg::index(), typename Arg::type>;

			template<typename... Args>
			struct PGSQLArgsHolder_str : private PGwrapped<Args>...{

				PGSQLArgsHolder_str(const PGSQLArgsHolder_str&) = delete;
				PGSQLArgsHolder_str(PGSQLArgsHolder_str&&) = delete;
				
				const std::vector<const char*> param_values;
				const std::vector<int> param_lengths;
				const std::vector<int> param_formats{one<Args>::value...};
				PGSQLArgsHolder_str(const typename Args::type& ...args)
					:PGwrapped<Args>(args)...,
					param_values{{PGwrapped<Args>::pg_data...}},
					param_lengths{{((int)PGwrapped<Args>::pg_size)...}}
					{}
			};

			template<typename... Args>
			using PGSQLArgsHolder = mutils::argswrapper<dummy_client, PGSQLArgsHolder_str,Args...>;

		}}}

