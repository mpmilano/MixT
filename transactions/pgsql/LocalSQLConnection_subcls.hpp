#pragma once

namespace myria { namespace pgsql {
		namespace local{

			template<Level l> class LocalSQLConnection : public LocalSQLConnection_super {
			public:
				LocalSQLConnection(){
					pgtransaction tr{*this,0};
					if (l == Level::strong){
						tr.exec_async(noop,"set search_path to \"BlobStore\",public");
						tr.exec_async(noop,"SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL SERIALIZABLE");
					}
					else {
						tr.exec_async(noop,"set search_path to causalstore,public");
						tr.exec_async(noop,"SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL REPEATABLE READ");
					}
					tr.commit(noop);
				}
			};
			
			template<typename... Types>
			void LocalSQLConnection_super::prepare(const std::string &name, const std::string &statement){
				pgtransaction tr{*this,0};
				tr.prepare(name, statement);
				tr.commit(noop);
			}
			
		}}}
