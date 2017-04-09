#include "Tombstone.hpp"

namespace myria { namespace tracker {
		bool operator==(const Tombstone& a, const Tombstone& b){
			return a.nonce == b.nonce &&
				a.ip_addr == b.ip_addr &&
				a.portno == b.portno;
		}
}}
