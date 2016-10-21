//oh look, a source file! We remember those.
#include "SQLConnection.hpp"
#include <pqxx/pqxx>
#include <arpa/inet.h>
#include "SQLStore_impl.hpp"
#include "SQLTransaction.hpp"
#include "Tracker_common.hpp"
#include "SQLCommands.hpp"
#include "SQLStore.hpp"
#include "Ends.hpp"
#include "Ostreams.hpp"
#include "SafeSet.hpp"
#include <stdio.h>
#include <unistd.h>
#include <string.h> /* for strncpy */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>



namespace myria{ namespace pgsql {

		using namespace pqxx;
		using namespace std;
		using namespace mtl;
		using namespace tracker;
		using namespace mutils;
		
		unsigned int get_eth0_addr(){
			int fd;
			struct ifreq ifr;
			
			fd = socket(AF_INET, SOCK_DGRAM, 0);
			
			/* I want to get an IPv4 IP address */
			ifr.ifr_addr.sa_family = AF_INET;
			
			/* I want IP address attached to "eth0" */
			strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
			
			ioctl(fd, SIOCGIFADDR, &ifr);
			
			close(fd);
			
			/* display result */
			std::string ip{inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr)};
			auto ret = decode_ip(ip);
			return ((unsigned int*)(&ret))[0];
		}

		bool SQLConnection::in_trans() const{
			return current_trans;
		}
	}
}
