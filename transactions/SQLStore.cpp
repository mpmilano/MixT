//oh look, a source file! We remember those.
#include <pqxx/pqxx>
#include <arpa/inet.h>
#include "SQLStore_impl.hpp"
#include "SQLTransaction.hpp"
#include "Tracker_common.hpp"
#include "SQLCommands.hpp"
#include "SQLStore.hpp"
#include "Ends.hpp"
#include "Ostreams.hpp"


using namespace pqxx;
using namespace std;
using Internals = SQLStore_impl::GSQLObject::Internals;

namespace {
	bool created_causal = false;
	bool created_strong = false;
}

const std::string& table_name(Table t){
	static const std::string bs = "\"BlobStore\"";
	static const std::string is = "\"IntStore\"";
	switch (t){
	case Table::BlobStore : return bs;
	case Table::IntStore : return is;
	};
	assert(false && "you always knew adding new tables would be a pain");
}

SQLStore_impl::SQLStore_impl(GDataStore &store, int instanceID, Level l)
    :_store(store),clock{{0,0,0,0}},level(l),default_connection{new SQLConnection(instanceID)} {
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
		->exec(l == Level::strong ?
			   "set search_path to \"BlobStore\",public"
			   : "set search_path to causalstore,public");
	((SQLTransaction*)t.get())
		->exec(l == Level::strong ?
			   "SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL SERIALIZABLE"
			   : "SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL REPEATABLE READ");
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
	std::array<int,4> causal_vers;
    Internals(Table table, int key, int size,
			  SQLStore_impl& store,char* buf, SQLTransaction* ctx)
        :table(table),key(key),size(size),store_id(store.instance_id()),level(store.level),_store(store),
		 buf1(buf),curr_ctx(ctx),vers(-1),causal_vers{{-1,-1,-1,-1}}
		{}
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

}

SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl &ss, Table t, int id, int size)
    :i(new Internals{t,id,size,ss,nullptr,nullptr}){
	i->buf1 = (char*) malloc(size);
	assert(load());
}

namespace {

	Internals* internals_from_size_ss_id(const Table t, SQLStore_impl& ss, int id){
		auto trans_owner = enter_store_transaction(ss);
		auto *trans = trans_owner.second;
		int size = -1;
		if (t == Table::BlobStore){
			result r = cmds::check_size(ss.level,*trans,t,id);
			assert(size == -1);
			assert(r.size() > 0);
			if (!(r[0][0].to(size))){
				std::cerr << "trying to find existing object: " << id << " from table " << table_name(t) << " (in " << ss.level << " cluster) had no size!" << std::endl;
			};
			assert(size != -1);
		}
		else if (t == Table::IntStore) size = sizeof(int);
        return new Internals{t,id,size,ss,
				(char*) malloc(size),nullptr};
	}
}

//existing object
SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl& ss, Table t, int id)
	:i{internals_from_size_ss_id(t,ss,id)} {}

//"named" object
SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl& ss, Table t, int id, const vector<char> &c)
    :i{new Internals{t,id,(int)c.size(),ss,
			(char*) malloc(c.size()),nullptr}}{
	assert(!ro_isValid());
	{
		auto trans_owner = enter_transaction(*this);
		auto *trans = trans_owner.second;

		if (t == Table::BlobStore){
			binarystring blob(&c.at(0),c.size());
			cmds::initialize_with_id(ss.level,*trans,t,ss.default_connection->repl_group,id,ss.clock,blob);
		}
		else if (t == Table::IntStore){
			cmds::initialize_with_id(ss.level,*trans,t,ss.default_connection->repl_group,id,ss.clock,((int*)c.data())[0]);
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
		//level doesn't matter here for now.
		return cmds::obj_exists(Level::undef,*owner,id).size() > 0;
	}
}

bool SQLStore_impl::exists(int id) {
    auto owner = enter_store_transaction(*this);
    return obj_exists(id,owner.second);
}

void SQLStore_impl::remove(int id) {
	cmds::remove(level,*small_transaction(*this),Table::BlobStore,id);
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

	int ip_to_group(unsigned int i){
		static_assert(sizeof(unsigned int) == 4,"Need a 32-bit type here");
		if (i == 0) return 1;
		else {
			int ret = (((unsigned char*)(&i))[3] % 4) + 1;
			assert(ret <= 10 && ret > 0);
			return ret;
		}
	}
}


SQLStore_impl::SQLConnection::SQLConnection(int ip):ip_addr(ip),repl_group(ip_to_group(ip)),conn{std::string("host=") + string_of_ip(ip)}{
	assert(conn.is_open());
	std::cout << string_of_ip(ip) << std::endl;
}

int SQLStore_impl::GSQLObject::store_instance_id() const {
	if (i->level == Level::strong)
		assert(&SQLStore<Level::strong>::inst(i->store_id) == &i->_store);
	else if (i->level == Level::causal){
		assert(&SQLStore<Level::causal>::inst(i->store_id) == &i->_store);
	}
	return i->store_id;
}

int SQLStore_impl::GSQLObject::name() const {
	return this->i->key;
}

const std::array<int,NUM_CAUSAL_GROUPS>& SQLStore_impl::GSQLObject::timestamp() const {
	return this->i->causal_vers;
}

void SQLStore_impl::GSQLObject::save(){
	char *c = obj_buffer();
	auto owner = enter_transaction(*this);
	auto trans = owner.second;

#define upd_23425(x...) cmds::update_data(i->_store.level,*trans,i->table,i->_store.default_connection->repl_group,i->key,i->_store.clock,x)
	
	if (i->table == Table::BlobStore){
		binarystring blob(c,i->size);
		upd_23425(blob);
	}
	else if (i->table == Table::IntStore){
		upd_23425(((int*)c)[0]);
	}

	if (i->_store.level == Level::strong){
		cmds::select_version(i->_store.level, *trans,i->table,i->key,i->vers);
	}
	else if (i->_store.level == Level::causal){
		cmds::select_version(i->_store.level, *trans,i->table,i->key,i->causal_vers);
		//I'm not actually contributing my counter from my clock to the version,
		//so there's no need to update it here. 
		//i->_store.clock = ends::max(i->_store.clock,i->causal_vers);
	}
}

char* SQLStore_impl::GSQLObject::load(){
	char* c = obj_buffer();
	auto owner = enter_transaction(*this);
	auto trans = owner.second;
	bool store_same = false;
	if (i->_store.level == Level::causal){
		auto old = i->causal_vers;
		cmds::select_version(i->_store.level, *trans,i->table,i->key,i->causal_vers);
		store_same = ends::is_same(old,i->causal_vers);
		if (!store_same) i->_store.clock = max(i->_store.clock,i->causal_vers);
		assert(i->_store.clock[2] < 30);
	}
	else if (i->_store.level == Level::strong){
		auto old = i->vers;
		int newi = -12;
		cmds::select_version(i->_store.level, *trans,i->table,i->key,newi);
		store_same = (old == newi);
		i->vers = newi;
	}

	if (store_same) return c;
	else{
		result r = cmds::select_data(i->_store.level,*trans,i->table,i->key);
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
	auto owner = enter_transaction(*this);
	cmds::increment(i->_store.level,
					*owner.second,
					i->table,
					i->_store.default_connection->repl_group,
					i->key,
					i->_store.clock);
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
                  SQLStore<Level::strong>::id() :
                  SQLStore<Level::causal>::id());
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
