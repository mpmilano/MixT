#pragma once
#include "postgresql/libpq-fe.h"
#include "Bytes.hpp"

//WOW are Oids literally the worst.

namespace myria { namespace pgsql { namespace local{

template<typename >
struct PGSQLinfo;

template<>
struct PGSQLinfo<long int> {
	static_assert(sizeof(long int) >= 8,"Wow postgres is irritating");
	/*bigint, but actually int8*/
	static constexpr Oid value = 20;
	static const char* pg_data(std::vector<char>& scratch_buf, const long int& li) {
		auto old_size = scratch_buf.size();
		scratch_buf.resize(old_size + sizeof(long int));
		long int *buf = (long int*) scratch_buf.data() + old_size;
		*buf = htobe64(li);
		return (char*) buf;
	}
	static long int from_pg(char const * const v){
		return be64toh(*((long int*)v));
	}
	static int pg_size(std::vector<char>&, const long int&){
		return sizeof(long int);
	}
};

template<>
struct PGSQLinfo<int> {
	static_assert(sizeof(int) >= 4,"Wow postgres is irritating");
	/*integer, but actually int4*/
	static constexpr Oid value = 23;
	static const char* pg_data(std::vector<char>& scratch_buf, const int& li) {
		auto old_size = scratch_buf.size();
		scratch_buf.resize(old_size + sizeof(int));
		int *buf = (int*) scratch_buf.data() + old_size;
		*buf = htobe32(li);
		return (char*) buf;
	}
	static int from_pg(char const * const v){
		return be32toh(*((int*)v));
	}
	static int pg_size(std::vector<char>&, const int&){
		return sizeof(int);
	}
};


//we're gonna call this a bytea
template<>
struct PGSQLinfo<mutils::Bytes> {
	/*bytea*/
	static constexpr Oid value = 17;
	static const char* pg_data(std::vector<char>&, mutils::Bytes& li) {
		return (char*) li.bytes;
	}
	static mutils::Bytes from_pg(std::size_t size, char const * const v){
		return mutils::Bytes{v,size};
	}
	static int pg_size(std::vector<char>&, mutils::Bytes& li){
		return li.size;
	}
};


template<typename> struct one { static constexpr int value = 1; };

template<typename T>
struct PGSQLinfo<T&> : public PGSQLinfo<T> {};

template<typename T>
struct PGSQLinfo<const T&> : public PGSQLinfo<T> {};

template<typename T>
struct PGSQLinfo<const T> : public PGSQLinfo<T> {};

		}}}
