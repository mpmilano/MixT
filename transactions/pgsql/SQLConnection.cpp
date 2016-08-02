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

		static constexpr std::size_t max_persistant_connections{200};
		
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
			:prepared(((std::size_t) TransactionNames::MAX),false),
			 conn{std::string("host=") + string_of_ip(ip_addr)} {
			static_assert(int{CAUSAL_GROUP} > 0, "errorr: did not set CAUSAL_GROUP or failed to 1-index");
			assert(conn.is_open());
			static std::size_t connections_open{0};
			std::cout << connections_open++ << std::endl;
			//std::cout << string_of_ip(ip) << std::endl;
		}
		const int SQLStore_impl::SQLConnection::repl_group;
		const unsigned int SQLStore_impl::SQLConnection::ip_addr;

		SQLStore_impl::SQLConnection::~SQLConnection(){
			conn.disconnect();
		}

		struct SQLStore_impl::LockedSQLConnection::Internals{
			std::unique_ptr<SQLConnection> mgr;
		};

		namespace{
			SafeSet<std::unique_ptr<SQLStore_impl::SQLConnection> > strong_mgr_instances;
			SafeSet<std::unique_ptr<SQLStore_impl::SQLConnection> > causal_mgr_instances;
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
			   

		SQLStore_impl::SQLConnection const *  SQLStore_impl::LockedSQLConnection::operator->() const {
			return i->mgr.get();
		}

		const SQLStore_impl::SQLConnection& SQLStore_impl::LockedSQLConnection::operator*() const {
			return *i->mgr;
		}
			   

		SQLStore_impl::LockedSQLConnection::~LockedSQLConnection(){
			if (i){
				assert(i->mgr->current_store);
				const auto level = i->mgr->current_store->level;
				i->mgr->current_store = nullptr;
				if (level == Level::causal) {
					if (causal_mgr_instances.size() <= max_persistant_connections)
						causal_mgr_instances.emplace(std::move(i->mgr));
				}
				else if (strong_mgr_instances.size() <= max_persistant_connections)
					strong_mgr_instances.emplace(std::move(i->mgr));
				delete i;
			}
		};

		SQLStore_impl::LockedSQLConnection SQLStore_impl::SQLConnection_t::lock(SQLStore_impl& store) const {
			std::unique_ptr<SQLStore_impl::SQLConnection> tmp{nullptr};
			while (!tmp)
				tmp = (l == Level::causal ? causal_mgr_instances : strong_mgr_instances)
					.template build_or_pop<SQLConnection*>([]() -> SQLConnection* {try {
								return new SQLConnection();
							}
							catch(const pqxx_exception& pe){
								std::cout << "Connection failed: " << pe.base().what() << std::endl;
								return nullptr;
							}});
			assert(!tmp->current_store);
			tmp->current_store = &store;
			return LockedSQLConnection{std::move(tmp)};
		}
	}
}
