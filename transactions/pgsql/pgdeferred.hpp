#pragma once
#include "LocalSQLConnection.hpp"
#include "pgresult.hpp"

namespace myria { namespace pgsql {
		namespace local{
			struct deferred_action{
				const std::function<void (pgresult)> on_complete;
				const std::string query_str;
				const std::function<void ()> query;
				deferred_action(
					std::function<void (pgresult)> on_complete,
					const std::string &query_str,
					std::function<void ()> query)
					:on_complete(on_complete),
					 query_str(query_str),
					 query(query){}
				bool submitted{false};
				deferred_action(const deferred_action&) = delete;
			};

			struct pgtransaction;

			struct deferred_transaction{
			private:
				bool no_fut_actions{false};
			public:
				bool no_future_actions() const;
				
				void indicate_no_future_actions();
				pgtransaction* trans;
				std::list<deferred_action> actions;
				deferred_transaction(pgtransaction& trans);

				~deferred_transaction();

				friend struct pgtransaction;
			};
		}}}
