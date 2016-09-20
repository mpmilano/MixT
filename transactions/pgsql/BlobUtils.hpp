#pragma once
#include <pqxx/binarystring.hxx>

namespace mutils{
	struct Bytes;
}

namespace myria {
	namespace pgsql {

		//these both step through Bytes<>
		std::size_t bytes_size(const pqxx::binarystring &b);
		
		std::size_t to_bytes(const pqxx::binarystring &b, char* v);

		pqxx::binarystring make_blob(const mutils::Bytes& b);
	}
}

namespace mutils{
	using myria::pgsql::bytes_size;
	using myria::pgsql::to_bytes;
	using myria::pgsql::make_blob;
}
