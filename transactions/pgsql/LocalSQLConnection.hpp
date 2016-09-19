#pragma once
#include "SQLConnection.hpp"

namespace myria { namespace pgsql {
		

		template<Level l>
		class LocalSQLConnection{

			using sizes_t = std::vector<std::size_t>;
			
			std::vector<bool> prepared;
			pqxx::connection conn;
			std::unique_ptr<LocalSQLTransaction<l> > current_trans{nullptr};
			mutils::connection* client_connection{nullptr};
			LocalSQLConnection()
				:prepared(((std::size_t) LocalTransactionNames::MAX),false)
				{}
		};
	}
}
