//oh look, a source file! We remember those.

#include "SQLStore.hpp"
#include <pqxx/pqxx>

using namespace pqxx;
using namespace std;

SQLStore::SQLStore():default_connection{make_unique<SQLConnection>()} {}

struct SQLStore::SQLConnection {
	bool in_trans = false;
	//hoping specifying nothing means
	//env will be used.
	connection conn;
	SQLConnection() = default;
	SQLConnection(const SQLConnection&) = delete;
};


struct SQLStore::SQLTransaction : public TransactionContext {
	SQLConnection& sql_conn;
	work trans;
	bool commit_on_delete = false;
	SQLTransaction(SQLConnection& c):sql_conn(c),trans(sql_conn.conn){
		sql_conn.in_trans = true;
		
		}
	SQLTransaction(const SQLTransaction&) = delete;
	
	bool commit() {
		sql_conn.in_trans = false;
		trans.commit();
		return true;
	}

	~SQLTransaction(){
		if (commit_on_delete) commit();
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
	SQLTransaction* curr_ctx;
};

SQLStore::GSQLObject::GSQLObject(int size)
	:i((new Internals
		{(char*) malloc(size),
				(char*) malloc(size),
				nullptr
				})){}


SQLStore::GSQLObject::GSQLObject(SQLStore::GSQLObject&& gso)
	:i(gso.i){gso.i = nullptr;}



SQLStore::GSQLObject::~GSQLObject(){
	if (i){
		free(i->buf1);
		free(i->buf2);
		delete i;
	}
}

void SQLStore::GSQLObject::setTransactionContext(TransactionContext* tc){
	assert(i->curr_ctx == nullptr || tc == nullptr);
	//we can't support nested transactions right now,
	//so it's really quite bad if there is already a transactions context here
	if (auto* ptr = dynamic_cast<SQLStore::SQLTransaction*>(tc)){
		i->curr_ctx = ptr;
	} else assert(false && "Error: gave SQLObject wrong kind of TransactionContext");
}

TransactionContext* SQLStore::GSQLObject::currentTransactionContext(){
	return i->curr_ctx;
}

const GDataStore& SQLStore::GSQLObject::store() const {
	return SQLStore::inst();
}

GDataStore& SQLStore::GSQLObject::store() {
	return SQLStore::inst();
}

void SQLStore::GSQLObject::save(){
	char *c = obj_buffer();
	unique_ptr<TransactionContext> t_owner;
	//this is always safe; we can't construct any others in here.
	SQLTransaction *trns = (SQLTransaction *) currentTransactionContext();
	if (!trns){
		auto ptr = inst().begin_transaction();
		trns = (SQLTransaction*) ptr.get();
		t_owner = std::move(ptr);
		trns->commit_on_delete = true;
	}
	if (/*brand-new-store*/ true)
		trns->trans.exec("insert into BlobStore values (?)");
	else trns->trans.exec("update BlobStore set ?=? where ?=?");
	//do stuff, transaction commits on destruction
}
