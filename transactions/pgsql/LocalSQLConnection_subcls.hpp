#pragma once

namespace myria { namespace pgsql {
		namespace local{

			template<Level l> class LocalSQLConnection : public LocalSQLConnection_super {
			public:
				LocalSQLConnection(whendebug(std::ofstream& log_file))
					whendebug(:LocalSQLConnection_super(log_file)){
					pgtransaction *tr{nullptr};
					std::unique_ptr<pgtransaction> tr_o;
					if (transactions.size() > 0 && !transactions.back().no_future_actions()){
						tr = transactions.back().trans;
					}
					else{
						tr_o.reset(new pgtransaction(*this,0));
						tr = tr_o.get();
					}

					if (l == Level::strong){
						tr->exec_async(noop,"set search_path to \"BlobStore\",public");
						tr->exec_async(noop,"SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL SERIALIZABLE");
					}
					else {
						tr->exec_async(noop,"set search_path to causalstore,public");
						tr->exec_async(noop,"SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL REPEATABLE READ");
					}
					if (tr_o) tr_o->commit(noop);
				}
			};
			
			template<typename... Types>
			void LocalSQLConnection_super::prepare(const std::string &name, const std::string &statement){
				pgtransaction *tr{nullptr};
				std::unique_ptr<pgtransaction> tr_o;
				if (transactions.size() > 0 && !transactions.back().no_future_actions()){
					tr = transactions.back().trans;
				}
				else{
					tr_o.reset(new pgtransaction(*this,1));
					tr = tr_o.get();
				}
				tr->prepare(name, statement);
				if (tr_o) tr_o->commit(noop);
			}
			
		}}}
