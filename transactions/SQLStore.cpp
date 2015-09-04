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

namespace {
	unique_ptr<SQLStore::SQLTransaction> small_transaction(SQLStore &store) {
		unique_ptr<SQLStore::SQLTransaction> owner
			((SQLStore::SQLTransaction*)store.begin_transaction().release());
		owner->commit_on_delete = true;
		return move(owner);
	}
}

struct SQLStore::GSQLObject::Internals{
	const int key;
	const int size;
	char* buf1;
	SQLTransaction* curr_ctx;
	int vers;

};


SQLStore::GSQLObject::GSQLObject(SQLStore::GSQLObject&& gso)
	:i(gso.i){gso.i = nullptr;}

SQLStore::GSQLObject::GSQLObject(const vector<char> &c){
	int size = c.size();
	int id = -1;
	{
		auto trans = small_transaction(inst());
		trans->sql_conn.conn.prepare
			("InitializeBlobData","INSERT INTO BlobStore (data) VALUES ($1)");
		//DO I need to prepare before I start the trans?
		binarystring blob(&c.at(0),size);
		trans->trans.prepared("InitializeBlobData")(blob).exec();
		result r = trans->trans.exec("select ID from BlobStore where ID = max(ID)");
		assert(r[0][0].to(id));
		assert(id != -1);
	}
	char* b1 = (char*) malloc(size);
	memcpy(b1,&c.at(0),size);
	this->i = new Internals{id,size,b1,nullptr,0};
}

SQLStore::GSQLObject::~GSQLObject(){
	if (i){
		free(i->buf1);
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

namespace{
	pair<unique_ptr<SQLStore::SQLTransaction>, SQLStore::SQLTransaction*>
		enter_transaction(SQLStore::GSQLObject& gso){
		unique_ptr<SQLStore::SQLTransaction> t_owner;
			//this is always safe; we can't construct any others in here.
		SQLStore::SQLTransaction *trns =
			(SQLStore::SQLTransaction *) gso.currentTransactionContext();
			if (!trns){
				t_owner = small_transaction(SQLStore::inst());
				trns = t_owner.get();
			}
			return make_pair(move(t_owner),trns);
	}
}

void SQLStore::GSQLObject::save(){
	char *c = obj_buffer();
	auto owner = enter_transaction(*this);
	auto trans = owner.second;
	int vers = -1;
	auto r = trans->trans.exec(
		string("select Version from BlobStore where ID=") + to_string(i->key));
	assert(r[0][0].to(vers));
	assert(vers != -1);
	trans->sql_conn.conn.prepare
		("UpdateBlobData",
		 string(
		  "update BlobStore set data=$1,Version=$2 where ID=" + to_string(i->key)));
	binarystring blob(c,i->size);
	trans->trans.prepared("UpdateBlobData")(blob)(vers).exec();
	i->vers = vers;
}

char* SQLStore::GSQLObject::load(){
	char* c = obj_buffer();
	auto owner = enter_transaction(*this);
	auto trans = owner.second;
	int store_vers = -1;
	auto r = trans->trans.exec(
		string("select Version from BlobStore where ID=") + to_string(i->key));
	assert(r[0][0].to(store_vers));
	assert(store_vers != -1);
	if (store_vers == i->vers) return c;
	else{
		i->vers = store_vers;
		result r = trans->trans.exec
			(string("select data from BlobStore where ID = ") + to_string(i->key));
		binarystring bs(r[0][0]);
		assert(bs.size() == i->size);
		memcpy(c,bs.data(),i->size);
		return c;
	}
}
