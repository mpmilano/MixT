//oh look, a source file! We remember those.

#include "SQLStore_impl.hpp"
#include "SQLTransaction.hpp"
#include "Tracker_common.hpp"
#include "SQLCommands.hpp"
#include "SQLStore.hpp"
#include <pqxx/pqxx>
#include <arpa/inet.h>

using namespace pqxx;
using namespace std;
using Internals = SQLStore_impl::GSQLObject::Internals;

namespace {
	bool created_causal = false;
	bool created_strong = false;
}

SQLStore_impl::SQLStore_impl(GDataStore &store, int instanceID, Level l)
    :_store(store),level(l),default_connection{new SQLConnection(instanceID)} {
	if (l == Level::strong) {
		assert(!created_strong);
		created_strong = true;
	}
	if (l == Level::causal) {
		assert(!created_causal);
		created_causal = true;
	}
    auto t = begin_transaction();
	((SQLTransaction*)t.get())
		->exec("set search_path to \"BlobStore\",public");
	assert(t->commit());
}

unique_ptr<TransactionContext> SQLStore_impl::begin_transaction() {
	assert(default_connection->in_trans == false &&
		   "Concurrency support doesn't exist yet."
		);
    return unique_ptr<TransactionContext>(new SQLTransaction(_store,*default_connection));
	
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
	const Table table;
	const int key;
	const int size;
	const int store_id;
	const Level level;
	SQLStore_impl &_store;
	char* buf1;
	SQLTransaction* curr_ctx;
	int vers;
	constexpr static auto select_max_id(Table t) {
		return ::select_max_id(t);
	}
	string select_vers;
	string select_data;
	string update_data;
    Internals(Table table, int key, int size,
			  SQLStore_impl& store,char* buf, SQLTransaction* ctx, int vers)
        :table(table),key(key),size(size),store_id(store.instance_id()),level(store.level),_store(store),
		 buf1(buf),curr_ctx(ctx),vers(vers)
		,select_vers(
			cmds::select_version(table) + to_string(key))
		,select_data(cmds::select_data(table) +
					 to_string(key))
		,update_data( cmds::update_data(table)
                      + to_string(key)){
        std::cout << "Made an internals with store_id" << store_id << std::endl;
    }
};

SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl::GSQLObject&& gso)
	:i(gso.i){gso.i = nullptr;}

namespace{

	auto enter_store_transaction(SQLStore_impl& store){
		unique_ptr<SQLTransaction> t_owner;
		SQLTransaction *trns = nullptr;
		if ((store).default_connection->in_trans == false){
			t_owner = small_transaction(store);
			trns = t_owner.get();
		}
		else trns = (store).default_connection->current_trans;
		return make_pair(move(t_owner),trns);
	}
	
	pair<unique_ptr<SQLTransaction>, SQLTransaction*>
		enter_transaction(SQLStore_impl::GSQLObject& gso){
		SQLTransaction *trns =
			(SQLTransaction *) gso.currentTransactionContext();
		if (!trns){
			return enter_store_transaction(gso.store());
		}
		else return make_pair(unique_ptr<SQLTransaction>{nullptr},trns);
	}
	/*
	Internals* init_internals(Table t, SQLStore_impl &ss, const vector<char> &c){
		int size = c.size();
		int id = -1;
		{
			auto trans_owner = enter_store_transaction(ss);
			auto *trans = trans_owner.second;

			if (t == Table::BlobStore){
				binarystring blob(&c.at(0),size);
				trans->prepared("InitializeData"
								,cmds::initialize_data(t),
								blob);
			}
			else if (t == Table::IntStore){
				trans->prepared("InitializeData"
								,cmds::initialize_data(t),
								((int*)c.data())[0]);
			}
			result r = trans->exec(Internals::select_max_id(t));
			
			assert(r.size() > 0);
			assert(r[0][0].to(id));
			assert(id != -1);
		}
		char* b1 = (char*) malloc(size);
		memcpy(b1,&c.at(0),size);
	
        return new Internals{t,id,size,ss,b1,nullptr,0};
		} //*/
}

SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl &ss, Table t, int id, int size)
    :i(new Internals{t,id,size,ss,nullptr,nullptr,-1}){
	i->buf1 = (char*) malloc(size);
	assert(load());
}

namespace {

	Internals* internals_from_size_ss_id(Table t, SQLStore_impl& ss, int id){
		auto trans_owner = enter_store_transaction(ss);
		auto *trans = trans_owner.second;
		int size = -1;
		if (t == Table::BlobStore){
			result r = trans->prepared(
				"CheckSize",
				cmds::check_size(t),id);
			assert(size == -1);
			assert(r.size() > 0);
			assert(r[0][0].to(size));
			assert(size != -1);
		}
		else if (t == Table::IntStore) size = sizeof(int);
        return new Internals{t,id,size,ss,
				(char*) malloc(size),nullptr,-1};
	}
}

//existing object
SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl& ss, Table t, int id)
	:i{internals_from_size_ss_id(ss,id)} {}

//"named" object
SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl& ss, Table t, int id, const vector<char> &c)
    :i{new Internals{t,id,(int)c.size(),ss,
			(char*) malloc(c.size()),nullptr,0}}{
	assert(!ro_isValid());
	{
		auto trans_owner = enter_transaction(*this);
		auto *trans = trans_owner.second;

		if (t == Table::BlobStore){
			binarystring blob(&c.at(0),c.size());
			trans->prepared("InitializeData",
							initialize_with_id(t),
							id,blob);
		}
		else if (t == Table::IntStore){
			trans->prepared("InitializeData",
							initialize_with_id(t),
							id,((int*)c.data())[0]);
		}
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
		return owner->prepared("Exists",cmds::obj_exists(),id).size() > 0;
	}
}

bool SQLStore_impl::exists(int id) {
	return obj_exists(id,small_transaction(*this));
}

void SQLStore_impl::remove(int id) {
	small_transaction(*this)->exec(cmd::remove(Table::BlobStore) + to_string(id));
	small_transaction(*this)->exec(cmd::remove(Table::IntStore) + to_string(id));
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

int SQLStore_impl::instance_id() const{
    return default_connection->ip_addr;
}

namespace {
    std::string string_of_ip(unsigned int i){
        if (i == 0) return "127.0.0.1";
        else {
            in_addr a;
            char str[INET_ADDRSTRLEN];
            a.s_addr = i;
            assert(a.s_addr == i);
            inet_ntop(AF_INET,&a,str,INET_ADDRSTRLEN);
            return std::string(str);
        }
    }
}


SQLStore_impl::SQLConnection::SQLConnection(int ip):ip_addr(ip),conn{std::string("host=") + string_of_ip(ip)}{}

int SQLStore_impl::GSQLObject::store_instance_id() const {
	if (i->level == Level::strong)
		assert(&SQLStore<Level::strong>::inst(i->store_id) == &i->_store);
	else if (i->level == Level::causal){
		//note: there's a independence-of-causal-stores
		//bug here, relating to IDs somehow. Must investigate.
		assert(&SQLStore<Level::causal>::inst(i->store_id) == &i->_store);
	}
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
	if (i->table == Table::BlobStore){
		binarystring blob(c,i->size);
		trans->prepared("UpdateData",i->update_data,blob,vers);
	}
	else trans->prepared("UpdateData",i->update_data,((int*)c)[0],vers);
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
		if (i->table == Table::BlobStore){
			binarystring bs(r[0][0]);
			assert(bs.size() == i->size);
			memcpy(c,bs.data(),i->size);
		}
		else if (i->table == Table::IntStore) {
			int res = -1;
			assert(r[0][0].to(res));
			((int*)c)[0] = res;
		}
		return c;
	}
}

void SQLStore_impl::GSQLObject::increment(){
	assert(false && "Todo: implement");
}

bool SQLStore_impl::GSQLObject::ro_isValid() const {
	auto owner = enter_transaction(*const_cast<GSQLObject*>(this));
	return obj_exists(i->key,owner.second);
}

char* SQLStore_impl::GSQLObject::obj_buffer() {
	return i->buf1;
}

int SQLStore_impl::GSQLObject::bytes_size() const {
	return sizeof(int)*4 + sizeof(Level) + sizeof(Table);
}

int SQLStore_impl::GSQLObject::to_bytes(char* c) const {
	//TODO: this is not symmetric! That is a bad design! Bad!
	int* arr = (int*)c;
    arr[0] = (i->level == Level::strong ?
                  SQLStore<Level::strong>::ds_id_nl() :
                  SQLStore<Level::causal>::ds_id_nl());
	arr[1] = i->key;
	arr[2] = i->size;
	arr[3] = i->store_id;
	Level* arrl = (Level*) (arr + 4);
	arrl[0] = i->level;
	Table* arrt = (Table*) (arrl + 1);
	arrt[0] = i->table;
	return this->bytes_size();
}

SQLStore_impl::GSQLObject SQLStore_impl::GSQLObject::from_bytes(char *v){
	int* arr = (int*)v;
	//arr[0] has already been used to find this implementation
	Level* arrl = (Level*) (arr + 3);
	Table* arrt = (Table*) (arrl + 1);
	//of from_bytes
	Level lvl = arrl[0];
	if (lvl == Level::strong){
		return GSQLObject(SQLStore<Level::strong>::inst(arr[2]),
						  arrt[0],arr[0],arr[1]);
	} else {
		return GSQLObject(SQLStore<Level::causal>::inst(arr[2]),
						  arrt[0],arr[0],arr[1]);
	}
}

SQLStore_impl::~SQLStore_impl(){
	delete default_connection;
}
