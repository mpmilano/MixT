#include "oid_translator.hpp"
namespace myria { namespace pgsql { namespace local{
			
			constexpr Oid PGSQLinfo<long int>::value;
			constexpr std::size_t PGSQLinfo<long int>::pg_size;
			constexpr Oid PGSQLinfo<mutils::Bytes>::value;
			constexpr Oid PGSQLinfo<bool>::value;
			constexpr std::size_t PGSQLinfo<bool>::pg_size;
		}}}
