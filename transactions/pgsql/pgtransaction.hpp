#pragma once
#pragma once

#include "LocalSQLConnection.hpp"
namespace myria { namespace pgsql {
		namespace local{

			struct pgresult;

			struct pgtransaction {
			private:
				bool no_fut_actions{false};
			public:

				bool no_future_actions() const ;

				void indicate_no_future_actions();
				
				const std::size_t transaction_id;
				LocalSQLConnection_super &conn;
				deferred_transaction* my_trans;
				pgtransaction(LocalSQLConnection_super &conn, const std::size_t tid);

				pgresult exec_sync (const std::string &command);

				void commit(std::function<void ()> action);

				void abort(std::function<void ()> action);

				template<typename F>
				void exec_async(F action, const std::string &command);
				
				template<typename F, typename... Args>
				void exec_prepared(F action, const std::string& name, const Args & ... args);
				
				pgtransaction(const pgtransaction&) = delete;

				~pgtransaction();
			};
		}}}
#include "pgtransaction_impl.hpp"
