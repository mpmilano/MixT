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
	
		bool SQLConnection::in_trans() const {
			return current_trans;
		}

		std::string get_hostname(Level l){
			return mutils::string_of_ip(
				l == Level::strong ? 
				mutils::get_strong_ip() :
				mutils::get_causal_ip()
				);
		}
		
		SQLConnection::SQLConnection(std::string host)
			:prepared(((std::size_t) TransactionNames::MAX),false),conn{std::string("host=") + host}{
			static_assert(int{CAUSAL_GROUP} > 0, "errorr: did not set CAUSAL_GROUP or failed to 1-index");
			assert(conn.is_open());
		}
		const int SQLConnection::repl_group;

	}
}
