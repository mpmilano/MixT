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

		WeakSQLConnection::WeakSQLConnection(weak_connection conn)
			:conn(std::move(conn)){}

		LockedSQLConnection WeakSQLConnection::lock(){
			return LockedSQLConnection{conn.lock()};
		}

		WeakSQLConnection::WeakSQLConnection(LockedSQLConnection &l)
			:conn(l.conn){}

		WeakSQLConnection::WeakSQLConnection(LockedSQLConnection &&l)
			:conn(l.conn){}

		LockedSQLConnection WeakSQLConnection::acquire_if_locked() const {
			return LockedSQLConnection(conn.acquire_if_locked());
		}

		LockedSQLConnection::LockedSQLConnection(locked_connection conn)
			:conn(std::move(conn)){}

		SQLTransaction_p LockedSQLConnection::current_trans() const {
			return (SQLTransaction*) conn->bonus_item;
		}
		
		void_p& LockedSQLConnection::current_trans_vp() {
			return conn->bonus_item;
		}
			
		bool LockedSQLConnection::in_trans(){
			return current_trans();
		}

		LockedSQLConnection::LockedSQLConnection(LockedSQLConnection&& o)
			:conn(std::move(o.conn)){}

	}
}
