//oh look, a source file! We remember those.
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

namespace myria{ namespace pgsql {

		using namespace pqxx;
		using namespace std;
		using namespace mtl;
		using namespace tracker;
		using namespace mutils;

		bool SQLConnection::in_trans() const{
			return current_trans;
		}
	}
}
