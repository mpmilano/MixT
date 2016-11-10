#include "oid_translator.hpp"
namespace myria { namespace pgsql { namespace local{
			
			const Oid PGSQLinfo<long int>::value;
			const Oid PGSQLinfo<int>::value;
			const Oid PGSQLinfo<mutils::Bytes>::value;
		}}}
