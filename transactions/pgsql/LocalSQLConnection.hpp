#pragma once
#include "SQLConnection.hpp"
#include "Basics.hpp"
#include "LocalSQLTransaction.hpp"

namespace myria { namespace pgsql {
		namespace local{

		template<Level l>
		class LocalSQLConnection{
		public:
			using sizes_t = std::vector<std::size_t>;
			
			std::vector<bool> prepared;
			pqxx::connection conn;
			LocalSQLConnection()
				:prepared(((std::size_t) LocalTransactionNames::MAX),false)
				{}
		};
		}
	}
}

#include "LocalSQLTransaction_impl.hpp"
