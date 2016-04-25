#include "sync_defs.h"
#include "mutils.hpp"
#include "Socket.hpp"

using namespace std;
using namespace mutils;

int main(){
	
	std::vector<pair<int,shared_ptr<Socket> > > ip_addrs {
		{make_pair(decode_ip("128.84.105.150"), std::shared_ptr<Socket>()),
				/*make_pair(decode_ip("128.84.217.139"), std::shared_ptr<Socket>()),*/
				make_pair(decode_ip("128.84.105.142"), std::shared_ptr<Socket>()),
				make_pair(decode_ip("128.84.105.88"), std::shared_ptr<Socket>()),
				make_pair(decode_ip("128.84.105.81"), std::shared_ptr<Socket>()),
				make_pair(decode_ip("128.84.105.69"), std::shared_ptr<Socket>())}};

	try { 

		auto conn_causal = init_causal();
		auto conn_strong = init_strong();

		std::cout << "beginning loop" << std::endl;
		while (true){
			std::array<int,4> tmp;
			select_causal_timestamp(*conn_causal,tmp);
			for (auto &ip_addr : ip_addrs){
				try{
					if (!(ip_addr.second && ip_addr.second->valid())) {
						std::cout << "trying for: " << string_of_ip(ip_addr.first)
								  << std::endl;
						ip_addr.second.reset(
							new Socket(Socket::connect(ip_addr.first,9999)));
					}
					if (ip_addr.second->valid()) ip_addr.second->send(tmp);
				} catch (const SocketException&){
					continue;
				}
			}
		}
	}
	catch(const pqxx::pqxx_exception &r){
		std::cerr << r.base().what() << std::endl;
		assert(false && "exec failed");
	}
}
