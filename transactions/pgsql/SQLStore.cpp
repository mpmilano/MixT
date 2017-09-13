//oh look, a source file! We remember those.
#include <pqxx/pqxx>
#include <arpa/inet.h>
#include "pgsql/SQLStore_impl.hpp"
#include "pgsql/SQLTransaction.hpp"
#include "pgsql/SQLCommands.hpp"
#include "pgsql/SQLStore.hpp"
#include "tracker/Ends.hpp"
#include "Ostreams.hpp"
#include "pgsql/SQL_internal_utils.hpp"

namespace myria{ namespace pgsql {

		using namespace pqxx;
		using namespace std;
		using namespace mtl;
		using namespace tracker;
		using namespace mutils;
		using Internals = SQLStore_impl::GSQLObject::Internals;

		SQLStore_impl::SQLStore_impl(whenpool(GeneralSQLConnectionPool& pool) whennopool(const std::string &host),
																 GDataStore &store, /*int instanceID,*/ Level l)
			:_store(store),clock{{0,0,0,0}},level(l),default_connection{
				whenpool(pool.acquire_weak())
					whennopool(new SQLConnection(host))
					} {
				auto t = begin_transaction(whendebug("Setting up this new SQLStore; gotta configure search paths and stuff."));
				((SQLTransaction*)t.get())
					->exec(l == Level::strong ?
						   "set search_path to \"BlobStore\",public"
						   : "set search_path to causalstore,public");
				((SQLTransaction*)t.get())
					->exec(l == Level::strong ?
						   "SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL SERIALIZABLE"
						   : "SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL REPEATABLE READ");
				whendebug(auto cres = )t->store_commit();
				assert(cres);
			}

		unique_ptr<SQLTransaction> SQLStore_impl::begin_transaction(whendebug(const std::string &why))
		{
			assert(whenpool(!(default_connection.is_locked()))
						 whennopool(default_connection ));
			assert(whenpool(!(default_connection.lock()->in_trans()))
						 whennopool(!default_connection->in_trans())
						 && "Concurrency support doesn't exist yet."
				);
			return unique_ptr<SQLTransaction>(
				new SQLTransaction(*this,_store,whenpool(default_connection.lock()) whennopool(std::move(default_connection)) whendebug(,why)));
		}
		
		bool SQLStore_impl::in_transaction() const {
#ifndef NOPOOL
			try {
				auto conn = this->default_connection.acquire_if_locked();
				bool it = conn->in_trans();
				assert ([&](){
						bool ct = conn->current_trans;
						return (it ? ct : true);}());
				return it;
			}
			catch (const mutils::ResourceInvalidException&) {
				return false;
			}
#else
			return !this->default_connection;
#endif
			
		}

		bool SQLStore_impl::exists(SQLTransaction* ctx, Name id) {
			return obj_exists(id,ctx);
		}

		void SQLStore_impl::remove(SQLTransaction* ctx, Name id) {
			cmds::remove(level,*ctx,Table::BlobStore,id);
		}


		int SQLStore_impl::ds_id() const{
			return 2 + (int) level;
		}

		int SQLStore_impl::instance_id() const{
			constexpr auto ret = mutils::decode_ip(MY_IP);
			return ret;
		}





		
		SQLStore_impl& SQLInstanceManager_abs::inst(Level l){
			if (l == Level::strong) return this->inst_strong();
			else if (l == Level::causal) return this->inst_causal();
			else assert(false && "what?");
			struct die{}; throw die{};
		}




		SQLStore_impl::~SQLStore_impl(){
		}//*/

	}
}
