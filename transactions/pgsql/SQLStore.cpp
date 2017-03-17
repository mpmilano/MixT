//oh look, a source file! We remember those.
#include <arpa/inet.h>
#include "SQLStore_impl.hpp"
#include "SQLTransaction.hpp"
#include "Tracker_common.hpp"
#include "SQLCommands.hpp"
#include "SQLStore.hpp"
#include "Ends.hpp"
#include "Ostreams.hpp"
#include "SQL_internal_utils.hpp"

namespace myria{ namespace pgsql {

		std::ostream& operator<<(std::ostream& o, Level l){
			switch(l){
			case Level::strong:
				return o << "strong";
			case Level::causal:
				return o << "causal";
			}

		using namespace std;
		using namespace mtl;
		using namespace tracker;
		using namespace mutils;
		using Internals = SQLStore_impl::GSQLObject::Internals;

		
		void SQLStore_impl::init_common(){
			/*
				auto t = begin_transaction("Setting up this new SQLStore; gotta configure search paths and stuff.");
				((SQLTransaction*)t.get())
					->exec(level == Label<strong> ?
						   "set search_path to \"BlobStore\",public"
						   : "set search_path to causalstore,public");
				((SQLTransaction*)t.get())
					->exec(level == Label<strong> ?
						   "SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL SERIALIZABLE"
						   : "SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL REPEATABLE READ");
				auto cres = t->store_commit();
				assert(cres);//*/
		}
		
		SQLStore_impl::SQLStore_impl(SQLConnectionPool<Label<causal> >& pool, GDataStore &store /*int instanceID,*/ )
			:_store(store),level(Level::causal),clock{{0,0,0,0}},default_connection{pool.acquire()} {
				init_common();
			}

		SQLStore_impl::SQLStore_impl(SQLConnectionPool<Label<strong>>& pool, GDataStore &store /*int instanceID,*/ )
			:_store(store),level(Level::strong),clock{{0,0,0,0}},default_connection{pool.acquire()} {
				init_common();
			}

		unique_ptr<SQLTransaction> SQLStore_impl::begin_transaction(whendebug(const std::string &why))
		{
			assert(!(default_connection.is_locked() &&
					 default_connection.lock()->in_trans()) &&
				   "Concurrency support doesn't exist yet."
				);
			return unique_ptr<SQLTransaction>(
				new SQLTransaction(level,_store,default_connection.lock() whendebug(,why)));
		}
		
		bool SQLStore_impl::in_transaction() const {
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
		}

		bool SQLStore_impl::exists(Name id) {
			auto owner = enter_store_transaction(*this);
			return obj_exists(id,owner.second);
		}

		void SQLStore_impl::remove(Name id) {
			small_transaction(*this whendebug(,std::string("Trying to remove something of name ") + std::to_string(id)))
				->Del(id);
		}


		int SQLStore_impl::ds_id() const{
			return 2 + (int) level;
		}

		int SQLStore_impl::instance_id() const{
			constexpr auto ret = mutils::decode_ip(MY_IP);
			return ret;
		}


		SQLStore_impl::~SQLStore_impl(){
		}//*/

	}
}
