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
		  void check_error(std::size_t,LocalSQLConnection_super &conn, const std::string& command, int result);

			struct pgtransaction;

		class LocalSQLConnection_super {
		public:
			
			using sizes_t = std::vector<std::size_t>;
			
			std::vector<bool> prepared;
		  std::size_t connection_id = mutils::gensym();

			
			std::list<deferred_transaction> transactions;
#ifndef NDEBUG
			std::ostream& log_file;
#endif
			
			PGconn *conn;
			std::shared_ptr<bool> aborting{new bool{false}};
			LocalSQLConnection_super(whendebug(std::ostream& log_file));

			LocalSQLConnection_super(const LocalSQLConnection_super&) = delete;
			
			template<typename... Types>
			void prepare(const std::string &name, const std::string &statement);

			bool submit_new_transaction();
			void tick();
			
			int underlying_fd();

			virtual ~LocalSQLConnection_super();
			
		};
			
		}
	}
}
