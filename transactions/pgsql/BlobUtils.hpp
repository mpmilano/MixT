#pragma once
#include <binarystring.hxx>
#include "SerializationSupport.hpp"
#include "Bytes.hpp"

namespace myria {
	namespace pgsql {

		//these both step through Bytes<>
		std::size_t bytes_size(const pqxx::binarystring &b){
			return mutils::Bytes{b.data(), b.size()}.bytes_size();
		}
		
		std::size_t to_bytes(const pqxx::binarystring &b, char* v){
			return mutils::Bytes{b.data(), b.size()}.to_bytes(v);
		}

		pqxx::binarystring make_blob(const Bytes& b){
			return pqxx::binarystring{b.data,b.size};
		}
	}
}
