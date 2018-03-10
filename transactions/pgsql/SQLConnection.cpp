//oh look, a source file! We remember those.
#include <pqxx/pqxx>
#include <arpa/inet.h>
#include "pgsql/SQLStore_impl.hpp"
#include "pgsql/SQLTransaction.hpp"
#include "pgsql/SQLCommands.hpp"
#include "pgsql/SQLStore.hpp"
#include "tracker/Ends.hpp"
#include "Ostreams.hpp"
#include "mutils-containers/SafeSet.hpp"

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
			:prepared(((std::size_t) TransactionNames::MAX),false)
#ifndef NOSQLCONNECTION
			,conn{std::string("host=") + host}
#endif
		{
			(void) host;
			static_assert(int{CAUSAL_GROUP} > 0, "errorr: did not set CAUSAL_GROUP or failed to 1-index");
#ifndef NOSQLCONNECTION
			assert(conn.is_open());
#endif
		}
		const int SQLConnection::repl_group;

	}
}
