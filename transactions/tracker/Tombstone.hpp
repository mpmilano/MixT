#pragma once

namespace myria { namespace tracker {

		using Nonce = int;
		
    struct Tombstone {
			Nonce nonce;
			int ip_addr;
      int portno;
      Name name() const;

    };
		static_assert(std::is_pod<Tombstone>::value);
  }}
