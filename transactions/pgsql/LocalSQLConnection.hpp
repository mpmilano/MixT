#pragma once
#include "SQLConnection.hpp"
#include "Basics.hpp"
#include "postgresql/libpq-fe.h"
#include "oid_translator.hpp"
#include "pgdeferred.hpp"
#include <endian.h>

namespace myria { namespace pgsql {
		namespace local{


			extern const std::function<void ()> noop;
			class LocalSQLConnection_super;
			
			/*throws failure condition on error */
			void check_error(LocalSQLConnection_super &conn, const std::string& command, int result);

			struct pgtransaction;

		class LocalSQLConnection_super {
		public:
			
			using sizes_t = std::vector<std::size_t>;
			
			std::vector<bool> prepared;


			
			std::list<deferred_transaction> transactions;
			
			PGconn *conn;
			std::shared_ptr<bool> aborting{new bool{false}};
			LocalSQLConnection_super();

			LocalSQLConnection_super(const LocalSQLConnection_super&) = delete;
			
			template<typename... Types>
			void prepare(const std::string &name, const std::string &statement){
				Oid oids[] = {PGSQLinfo<Types>::value...};
				pgresult{statement,*this,PQprepare(conn,name.c_str(),statement.c_str(),sizeof...(Types),oids)};
			}

			void tick();
			
			int underlying_fd();

			virtual ~LocalSQLConnection_super();
			
		};
			
			template<Level l> class LocalSQLConnection : public LocalSQLConnection_super {};
		}
	}
}
