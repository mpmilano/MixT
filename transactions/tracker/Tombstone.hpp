#pragma once
#include "Basics.hpp"

namespace myria { namespace tracker {

		using Nonce = int;
		
    struct Tombstone {
			Nonce nonce;
			int ip_addr;
      int portno;
      Name name() const;

    };

		bool operator==(const Tombstone& a, const Tombstone& b){
			return a.nonce == b.nonce &&
				a.ip_addr == b.ip_addr &&
				a.portno == b.portno;
		}
		
		static_assert(std::is_pod<Tombstone>::value);
  }}
