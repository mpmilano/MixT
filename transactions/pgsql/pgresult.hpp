#pragma once

namespace myria { namespace pgsql {
		namespace local{
			class LocalSQLConnection_super;
			struct pgresult {
				const std::string command;
				LocalSQLConnection_super& conn;
				PGresult * res;
				~pgresult();
				pgresult(const pgresult&) = delete;
				pgresult(pgresult&&);
				pgresult(const std::string& command,
					   LocalSQLConnection_super &conn, PGresult *res);
			};
		}}}
