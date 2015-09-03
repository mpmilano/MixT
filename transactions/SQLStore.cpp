//oh look, a source file! We remember those.

#include "SQLStore.hpp"

//using namespace pqxx;
using namespace std;

SQLStore::SQLStore():default_connection{make_unique<SQLConnection>()} {}

struct SQLStore::SQLConnection {
		bool in_trans = false;
		SQLConnection() = default;
		SQLConnection(const SQLConnection&) = delete;
	};


struct SQLStore::SQLTransaction : public TransactionContext {
		SQLConnection& sql_conn;
		SQLTransaction(SQLConnection& c):sql_conn(c){
			sql_conn.in_trans = true;
		}

		bool commit() {
			sql_conn.in_trans = false;
			return true;
		}
		
	};


SQLStore& SQLStore::inst() {
		static SQLStore ss;
		return ss;
	}

unique_ptr<TransactionContext> SQLStore::begin_transaction() {
	assert(default_connection->in_trans == false &&
		   "Concurrency support doesn't exist yet."
		);
	return unique_ptr<TransactionContext>(new SQLTransaction(*default_connection));
	
}

struct SQLStore::GSQLObject::Internals{
	char* buf1;
	char* buf2;
};

SQLStore::GSQLObject::GSQLObject(int size)
	:i(*(new Internals{(char*) malloc(size), (char*) malloc(size)})){}
SQLStore::GSQLObject::~GSQLObject(){
	free(i.buf1);
	free(i.buf2);
	delete &i;
}
