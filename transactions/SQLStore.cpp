//oh look, a source file! We remember those.

#include "SQLStore.hpp"
#include <pqxx/pqxx>

using namespace pqxx;
using namespace std;

struct SQLTransaction;

struct SQLStore::SQLConnection {
	bool in_trans = false;
	SQLTransaction* current_trans = nullptr;
	
	//hoping specifying nothing means
	//env will be used.
	connection conn;
	SQLConnection() = default;
	SQLConnection(const SQLConnection&) = delete;
};

template<typename E>
auto exec_prepared_hlpr(E &e){
	return e.exec();
}

template<typename E, typename A, typename... B>
auto exec_prepared_hlpr(E &e, A&& a, B && ... b){
	auto fwd = e(forward<A>(a));
	return exec_prepared_hlpr(fwd,forward<B>(b)...);
}

struct SQLTransaction : public TransactionContext {
private:
	SQLStore::SQLConnection& sql_conn;
	work trans;
public:
	bool commit_on_delete = false;
	SQLTransaction(SQLStore::SQLConnection& c):sql_conn(c),trans(sql_conn.conn){
		assert(!sql_conn.in_trans);
		sql_conn.in_trans = true;
		sql_conn.current_trans = this;
		}
	
	SQLTransaction(const SQLTransaction&) = delete;

#define default_sqltransaction_catch					\
	catch(const pqxx_exception &r){						\
		commit_on_delete = false;						\
		std::cerr << r.base().what() << std::endl;		\
		assert(false && "exec failed");					\
	}

	
	template<typename Arg1, typename... Args>
	auto prepared(const string &name, const string &stmt, Arg1 && a1, Args && ... args){
		try{
			sql_conn.conn.prepare(name,stmt);
			auto fwd = trans.prepared(name)(forward<Arg1>(a1));
			return exec_prepared_hlpr(fwd,forward<Args>(args)...);
		}
		default_sqltransaction_catch
	}


	auto exec(const std::string &str){
		try{
			return trans.exec(str);
		}
		default_sqltransaction_catch
	}
	
	bool commit() {
		sql_conn.in_trans = false;
		sql_conn.current_trans = nullptr;
		trans.commit();
		return true;
	}

	GDataStore& store() {
		return SQLStore::inst();
	}

	list<SQLStore::GSQLObject*> objs;
	void add_obj(SQLStore::GSQLObject* gso){
		objs.push_back(gso);
	}

	~SQLTransaction(){
		if (commit_on_delete) {
			cout << "commit on delete" << endl;
			commit();
		}
		else sql_conn.in_trans = false;
		sql_conn.current_trans = nullptr;
		for (auto gso : objs)
			if (gso->currentTransactionContext() == this)
				gso->setTransactionContext(nullptr);
	}
	
};

SQLStore::SQLStore():default_connection{make_unique<SQLConnection>()} {
	auto t = begin_transaction();
	((SQLTransaction*)t.get())
		->exec("set search_path to \"BlobStore\",public");
	assert(t->commit());
	Tracker::global_tracker().registerStore(
		*this,
		[&](const auto &a, const auto &b){ return newObject<HandleAccess::all>(a,b);},
		[&](const auto &a){assert(false); return true;},
		[&](const auto &a, auto* inf){
			return existingObject<HandleAccess::all>(a,inf);});
}


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
	unique_ptr<SQLTransaction> small_transaction(SQLStore &store) {
		unique_ptr<SQLTransaction> owner
			((SQLTransaction*)store.begin_transaction().release());
		owner->commit_on_delete = true;
		return owner;
	}
}

struct SQLStore::GSQLObject::Internals{
	const int key;
	const int size;
	char* buf1;
	SQLTransaction* curr_ctx;
	int vers;
	constexpr static auto select_max_id() {
		return "select max(ID) from \"BlobStore\"";
	}
	string select_vers;
	string select_data;
	string check_existence;
	string update_data;
	Internals(int key, int size, char* buf, SQLTransaction* ctx, int vers)
		:key(key),size(size),buf1(buf),curr_ctx(ctx),vers(vers)
		,select_vers(
			string("select Version from \"BlobStore\" where ID=") + to_string(key))
		,select_data(string("select data from \"BlobStore\" where ID = ") +
					 to_string(key))
		,check_existence(string("select ID from \"BlobStore\" where ID = ") +
						 to_string(key) + " limit 1")
		,update_data( string("update \"BlobStore\" set data=$1,Version=$2 where ID=")
					  + to_string(key)){}

};

namespace{
	pair<unique_ptr<SQLTransaction>, SQLTransaction*>
		enter_transaction(SQLStore::GSQLObject& gso){
		unique_ptr<SQLTransaction> t_owner;
			//this is always safe; we can't construct any others in here.
		SQLTransaction *trns =
			(SQLTransaction *) gso.currentTransactionContext();
		if (!trns){
			if (((SQLStore&)gso.store()).default_connection->in_trans == false){
				t_owner = small_transaction(SQLStore::inst());
				trns = t_owner.get();
			}
			else trns = ((SQLStore&)gso.store()).default_connection->current_trans;
		}
		return make_pair(move(t_owner),trns);
	}
}


SQLStore::GSQLObject::GSQLObject(SQLStore::GSQLObject&& gso)
	:i(gso.i){gso.i = nullptr;}

SQLStore::GSQLObject::GSQLObject(const vector<char> &c){
	int size = c.size();
	int id = -1;
	{

		this->i = nullptr;
		auto trans_owner = enter_transaction(*this);
		auto *trans = trans_owner.second;
		
		binarystring blob(&c.at(0),size);
		trans->prepared("InitializeBlobData",
								  "INSERT INTO \"BlobStore\" (data) VALUES ($1)",
								  blob);
		result r = trans->exec(i->select_max_id());
		assert(r.size() > 0);
		assert(r[0][0].to(id));
		assert(id != -1);
	}
	char* b1 = (char*) malloc(size);
	memcpy(b1,&c.at(0),size);
	this->i = new Internals{id,size,b1,nullptr,0};
}

SQLStore::GSQLObject::GSQLObject(int id, int size)
	:i(new Internals{id,size,nullptr,nullptr,-1}){
	i->buf1 = (char*) malloc(size);
	assert(load());
}

SQLStore::GSQLObject::~GSQLObject(){
	if (i){
		free(i->buf1);
		delete i;
	}
}

void SQLStore::GSQLObject::setTransactionContext(TransactionContext* tc){
	assert(i->curr_ctx == nullptr || tc == nullptr);
	if (tc == nullptr) {
		i->curr_ctx = nullptr;
		return;
	}
	//we can't support nested transactions right now,
	//so it's really quite bad if there is already a transactions context here
	if (auto* ptr = dynamic_cast<SQLTransaction*>(tc)){
		i->curr_ctx = ptr;
		ptr->add_obj(this);
	} else assert(false && "Error: gave SQLObject wrong kind of TransactionContext");
}

TransactionContext* SQLStore::GSQLObject::currentTransactionContext(){
	if (i)
		return i->curr_ctx;
	else return nullptr;
}

const GDataStore& SQLStore::GSQLObject::store() const {
	return SQLStore::inst();
}

GDataStore& SQLStore::GSQLObject::store() {
	return SQLStore::inst();
}

void SQLStore::GSQLObject::save(){
	char *c = obj_buffer();
	auto owner = enter_transaction(*this);
	auto trans = owner.second;
	int vers = -1;
	auto r = trans->exec(i->select_vers);
	assert(r[0][0].to(vers));
	assert(vers != -1);
	binarystring blob(c,i->size);
	trans->prepared("UpdateBlobData",i->update_data,blob,vers);
	i->vers = vers;
}

char* SQLStore::GSQLObject::load(){
	char* c = obj_buffer();
	auto owner = enter_transaction(*this);
	auto trans = owner.second;
	int store_vers = -1;
	auto r = trans->exec(i->select_vers);
	assert(r[0][0].to(store_vers));
	assert(store_vers != -1);
	if (store_vers == i->vers) return c;
	else{
		i->vers = store_vers;
		result r = trans->exec(i->select_data);
		binarystring bs(r[0][0]);
		assert(bs.size() == i->size);
		memcpy(c,bs.data(),i->size);
		return c;
	}
}

bool SQLStore::GSQLObject::ro_isValid() const {
	auto owner = enter_transaction(*const_cast<GSQLObject*>(this));
	return owner.second->exec(i->check_existence).size() > 0;
}

char* SQLStore::GSQLObject::obj_buffer() {
	return i->buf1;
}

int SQLStore::GSQLObject::bytes_size() const {
	return sizeof(int)*3;
}

int SQLStore::GSQLObject::to_bytes(char* c) const {
	//TODO: this is not symmetric! That is a bad design! Bad!
	int* arr = (int*)c;
	arr[0] = id::value;
	arr[1] = i->key;
	arr[2] = i->size;
	return this->bytes_size();
}

SQLStore::GSQLObject SQLStore::GSQLObject::from_bytes(char *v){
	int* arr = (int*)v;
	return GSQLObject(arr[0],arr[1]);
}
