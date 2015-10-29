//oh look, a source file! We remember those.

#include "SQLStore_impl.hpp"
#include "SQLTransaction.hpp"
#include "Tracker_common.hpp"
#include "SQLStore.hpp"
#include <pqxx/pqxx>

using namespace pqxx;
using namespace std;

SQLStore_impl::SQLStore_impl(Level l):level(l),default_connection{new SQLConnection()} {
	auto t = begin_transaction();
	((SQLTransaction*)t.get())
		->exec("set search_path to \"BlobStore\",public");
	assert(t->commit());
}

unique_ptr<TransactionContext> SQLStore_impl::begin_transaction() {
	assert(default_connection->in_trans == false &&
		   "Concurrency support doesn't exist yet."
		);
	return unique_ptr<TransactionContext>(new SQLTransaction(*default_connection));
	
}

namespace {
	unique_ptr<SQLTransaction> small_transaction(SQLStore_impl &store) {
		unique_ptr<SQLTransaction> owner
			((SQLTransaction*)store.begin_transaction().release());
		owner->commit_on_delete = true;
		return owner;
	}
}

struct SQLStore_impl::GSQLObject::Internals{
	const int key;
	const int size;
	const int store_id;
	const Level level;
	SQLStore_impl &_store;
	char* buf1;
	SQLTransaction* curr_ctx;
	int vers;
	constexpr static auto select_max_id() {
		return "select max(ID) from \"BlobStore\"";
	}
	string select_vers;
	string select_data;
	string update_data;
	Internals(int key, int size, int store_id, Level l,
			  SQLStore_impl& store,char* buf, SQLTransaction* ctx, int vers)
		:key(key),size(size),store_id(store_id),level(l),_store(store),
		 buf1(buf),curr_ctx(ctx),vers(vers)
		,select_vers(
			string("select Version from \"BlobStore\" where ID=") + to_string(key))
		,select_data(string("select data from \"BlobStore\" where ID = ") +
					 to_string(key))
		,update_data( string("update \"BlobStore\" set data=$1,Version=$2 where ID=")
					  + to_string(key)){}
};

namespace{
	pair<unique_ptr<SQLTransaction>, SQLTransaction*>
		enter_transaction(SQLStore_impl::GSQLObject& gso){
		unique_ptr<SQLTransaction> t_owner;
			//this is always safe; we can't construct any others in here.
		SQLTransaction *trns =
			(SQLTransaction *) gso.currentTransactionContext();
		if (!trns){
			auto &store = gso.store();
			if ((store).default_connection->in_trans == false){
				t_owner = small_transaction(store);
				trns = t_owner.get();
			}
			else trns = (store).default_connection->current_trans;
		}
		return make_pair(move(t_owner),trns);
	}
}


SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl::GSQLObject&& gso)
	:i(gso.i){gso.i = nullptr;}

SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl& ss, const vector<char> &c){
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
	
	this->i = new Internals{id,size,ss.ds_id(),ss.level,ss,b1,nullptr,0};
}

SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl &ss, int id, int size)
	:i(new Internals{id,size,ss.ds_id(),ss.level,ss,nullptr,nullptr,-1}){
	i->buf1 = (char*) malloc(size);
	assert(load());
}

namespace {
	int blob_size(int key,SQLStore_impl::GSQLObject::GSQLObject &ctx){
		auto trans_owner = enter_transaction(ctx);
		auto *trans = trans_owner.second;
		int size = -1;
		result r = trans->prepared(
			"CheckSize",
			"select octet_length(data) from \"BlobStore\" where id = $1",key);
		assert(size == -1);
		assert(r.size() > 0);
		assert(r[0][0].to(size));
		assert(size != -1);
		return size;
	}
}

//existing object
SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl& ss, int id){
	int size = blob_size(id,*this);
	this->i = new Internals{id,size,ss.ds_id(),ss.level,ss,(char*) malloc(size),nullptr,-1};
}

//"named" object
SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl& ss, int id, const vector<char> &c)
	:i{new Internals{id,(int)c.size(),ss.ds_id(),ss.level,ss,
			(char*) malloc(c.size()),nullptr,0}}{
	assert(!ro_isValid());
	{
		auto trans_owner = enter_transaction(*this);
		auto *trans = trans_owner.second;
		
		binarystring blob(&c.at(0),c.size());
		trans->prepared("InitializeBlobData",
						"INSERT INTO \"BlobStore\" (id,data) VALUES ($1,$2)",
						id,blob);
	}
	memcpy(this->i->buf1, &c.at(0), c.size());
}

SQLStore_impl& SQLStore_impl::GSQLObject::store() {
	return i->_store;
}

namespace {
	//transaction context needs to be different sometimes
	template<typename Trans>
	bool obj_exists(int id, Trans owner){
		const static std::string q1 = "select ID from \"BlobStore\" where ID = ";
		const static std::string q2 = " limit 1";
		return owner->exec(q1 + to_string(id) + q2).size() > 0;
	}
}

bool SQLStore_impl::exists(int id) {
	return obj_exists(id,small_transaction(*this));
}

void SQLStore_impl::remove(int id) {
	const static std::string q1 = "delete from \"BlobStore\" where ID = ";
	small_transaction(*this)->exec(q1 + to_string(id));
}

SQLStore_impl::GSQLObject::~GSQLObject(){
	if (i){
		free(i->buf1);
		delete i;
	}
}

void SQLStore_impl::GSQLObject::setTransactionContext(TransactionContext* tc){
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
TransactionContext* SQLStore_impl::GSQLObject::currentTransactionContext(){
	if (i)
		return i->curr_ctx;
	else return nullptr;
}

int SQLStore_impl::ds_id() const{
	return 2 + (int) level;
}

int SQLStore_impl::GSQLObject::store_instance_id() const {
	return i->store_id;
}

int SQLStore_impl::GSQLObject::name() const {
	return this->i->key;
}

void SQLStore_impl::GSQLObject::save(){
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

char* SQLStore_impl::GSQLObject::load(){
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

bool SQLStore_impl::GSQLObject::ro_isValid() const {
	auto owner = enter_transaction(*const_cast<GSQLObject*>(this));
	return obj_exists(i->key,owner.second);
}

char* SQLStore_impl::GSQLObject::obj_buffer() {
	return i->buf1;
}

int SQLStore_impl::GSQLObject::bytes_size() const {
	return sizeof(int)*5;
}

int SQLStore_impl::GSQLObject::to_bytes(char* c) const {
	//TODO: this is not symmetric! That is a bad design! Bad!
	int* arr = (int*)c;
	arr[0] = i->store_id;
	arr[1] = i->key;
	arr[2] = i->size;
	arr[3] = i->store_id;
	arr[4] = (int) i->level;
	return this->bytes_size();
}

SQLStore_impl::GSQLObject SQLStore_impl::GSQLObject::from_bytes(char *v){
	int* arr = (int*)v;
	//arr[0] has already been used to find this implementation
	//of from_bytes
	Level lvl = (Level) arr[3];
	if (lvl == Level::strong){
		return GSQLObject(SQLStore<Level::strong>::inst(arr[2]),arr[0],arr[1]);
	} else {
		return GSQLObject(SQLStore<Level::causal>::inst(arr[2]),arr[0],arr[1]);
	}
}

SQLStore_impl::~SQLStore_impl(){
	delete default_connection;
}
