#pragma once
#include "Basics.hpp"
#include "mtl/mtlutils.hpp"

namespace myria {
namespace tracker {

using Nonce = long;

struct Tombstone {
  Nonce nonce;
  int ip_addr;
  int portno;
  Name name() const;
};

bool operator==(const Tombstone &a, const Tombstone &b);
bool operator<(const Tombstone &a, const Tombstone &b);

static_assert(std::is_pod<Tombstone>::value);
}
}
namespace mutils{
	template<>
	struct typename_str<myria::tracker::Tombstone>{
		static std::string f(){ return "Tombstone";}
	};
}
