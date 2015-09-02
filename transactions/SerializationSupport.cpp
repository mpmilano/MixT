#include "SerializationSupport.hpp"

int to_bytes(const ByteRepresentable& b, char* v){
	return b.to_bytes(v);
}

int bytes_size(const ByteRepresentable& b){
	return b.bytes_size();
}
