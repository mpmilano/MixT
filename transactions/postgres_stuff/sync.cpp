#include "sync_defs.h"
#include "mutils.hpp"
#include "Socket.hpp"

using namespace std;
using namespace mutils;

int main(){
	
	std::vector<pair<int,shared_ptr<Socket> > > ip_addrs {
		{
make_pair(decode_ip("128.84.105.158"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.107"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.103"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.148"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.88"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.72"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.70"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.118"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.92"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.140"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.128"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.81"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.77"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.98"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.153"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.90"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.122"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.121"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.66"), std::shared_ptr<Socket>()),
make_pair(decode_ip("128.84.105.135"), std::shared_ptr<Socket>())

					}};

	try { 

		auto conn_causal = init_causal();
		auto conn_strong = init_strong();

		std::cout << "beginning loop" << std::endl;
		while (true){
			std::array<unsigned long long,4> tmp;
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
