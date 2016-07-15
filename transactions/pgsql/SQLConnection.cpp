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
		
		bool SQLStore_impl::SQLConnection::in_trans(){
			if (current_trans){/*
				assert(con_guard.try_lock());
                                con_guard.unlock();*/
			}
			return current_trans;
		}

				SQLStore_impl::SQLConnection::SQLConnection()
			:prepared(((std::size_t) TransactionNames::MAX),false),conn{std::string("host=") + string_of_ip(ip_addr)}{
			static_assert(int{CAUSAL_GROUP} > 0, "errorr: did not set CAUSAL_GROUP or failed to 1-index");
			assert(conn.is_open());
			//std::cout << string_of_ip(ip) << std::endl;
		}
		const int SQLStore_impl::SQLConnection::repl_group;
		const unsigned int SQLStore_impl::SQLConnection::ip_addr;

		struct SQLStore_impl::LockedSQLConnection::Internals{
			std::unique_ptr<SQLConnection> mgr;
		};

		namespace{
			SafeSet<std::unique_ptr<SQLStore_impl::SQLConnection> > global_mgr_instances;
		}
		
		SQLStore_impl::LockedSQLConnection::LockedSQLConnection(std::unique_ptr<SQLConnection> p)
			:i(new Internals{std::move(p)}){
		}

		SQLStore_impl::SQLConnection* SQLStore_impl::LockedSQLConnection::operator->(){
			return i->mgr.get();
		}

		SQLStore_impl::SQLConnection& SQLStore_impl::LockedSQLConnection::operator*(){
			return *i->mgr;
		}
			   

		SQLStore_impl::SQLConnection const * const SQLStore_impl::LockedSQLConnection::operator->() const {
			return i->mgr.get();
		}

		const SQLStore_impl::SQLConnection& SQLStore_impl::LockedSQLConnection::operator*() const {
			return *i->mgr;
		}
			   

		SQLStore_impl::LockedSQLConnection::~LockedSQLConnection(){
			if (i){
				global_mgr_instances.emplace(std::move(i->mgr));
				delete i;
			}
		};

		SQLStore_impl::LockedSQLConnection SQLStore_impl::SQLConnection_t::operator->() const {
			return lock();
		}

		SQLStore_impl::LockedSQLConnection SQLStore_impl::SQLConnection_t::lock() const {
			return LockedSQLConnection{
				global_mgr_instances.template build_or_pop<SQLConnection*>(
					[](){return new SQLConnection();})};
		}

	}
}
