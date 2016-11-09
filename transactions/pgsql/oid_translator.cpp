#include "oid_translator.hpp"
namespace myria { namespace pgsql { namespace local{
			const Oid oid<long int>::value;
			const Oid oid<int>::value;
			const Oid oid<mutils::Bytes>::value;
		}}}
