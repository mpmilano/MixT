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

		bool operator==(const Tombstone& a, const Tombstone& b);
		
		static_assert(std::is_pod<Tombstone>::value);
  }}
