#pragma once

namespace myria { namespace tracker {
    struct Tombstone {
      const Nonce nonce;
      const int ip_addr;
      const int portno;
      Name name() const;
      
      /*
	int to_bytes(char* v) const;
	int bytes_size() const;
	static std::unique_ptr<Tombstone> from_bytes(DeserializationManager*,char const* v);
	Tombstone(Nonce n,int ip, int portno):nonce(n),ip_addr(ip),portno(portno){}
	Tombstone(const Tombstone& t):nonce(t.nonce),ip_addr(t.ip_addr),portno(t.portno){} //*/
    };
  }}
