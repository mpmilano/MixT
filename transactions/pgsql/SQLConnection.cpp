//oh look, a source file! We remember those.
#include "SQLConnection.hpp"
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
#include <sys/sysinfo.h>



namespace myria{ namespace pgsql {

		using namespace std;
		using namespace mtl;
		using namespace tracker;
		using namespace mutils;

		bool SQLConnection::in_trans() const{
			return current_trans;
		}

		struct sysinfo SQLConnection::collect_machine_stats(){
			constexpr char key{6};
			conn->send(key);
			return *conn->template receive<struct sysinfo>(
				nullptr,
				*conn->template receive<std::size_t>(nullptr,sizeof(std::size_t)));
		}

#ifndef NDEBUG
		template<>
		bool SQLConnectionPool<Level::strong>::constructed = false;
		template<>
		bool SQLConnectionPool<Level::causal>::constructed = false;
#endif
	}
}
