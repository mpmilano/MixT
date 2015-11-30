#include "SerializationSupport.hpp"

namespace mutils {

	int to_bytes(const ByteRepresentable& b, char* v){
		return b.to_bytes(v);
	}

	int bytes_size(const ByteRepresentable& b){
		return b.bytes_size();
	}

	int to_bytes(const std::string& b, char* v){
		strcpy(v,b.c_str());
		return b.length() + 1;
	}

	int bytes_size(const std::string& b){
		return b.length() + 1;
	}

}
