//oh look, a source file! We remember those.
#include <pqxx/pqxx>
#include <arpa/inet.h>
#include "SQLStore_impl.hpp"
#include "SQLTransaction.hpp"
#include "SQLCommands.hpp"
#include "SQLStore.hpp"
#include "Ends.hpp"
#include "Ostreams.hpp"
#include "SafeSet.hpp"
#include "SQL_internal_utils.hpp"

namespace myria{ namespace pgsql {

		namespace{
			constexpr int max_ver_check_size = 100;
		}
		
		using namespace pqxx;
		using namespace std;
		using namespace mtl;
		using namespace tracker;
		using namespace mutils;

		struct SQLStore_impl::GSQLObject::Internals{
			const Table table;
			const Name key;
			std::size_t size;
			const int store_id;
			const Level level;
			SQLStore_impl &_store;
			char* buf1;
			int vers;
			std::array<long long,4> causal_vers;
			Internals(Table table, Name key, int size,
					  SQLStore_impl& store,char* buf)
				:table(table),key(key),size(size),store_id(store.instance_id()),level(store.level),_store(store),
				 buf1(buf),vers(-1),causal_vers{{-1,-1,-1,-1}}
				{}
		};

		SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl::GSQLObject&& gso)
			:i(gso.i){gso.i = nullptr;}

				SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl &ss, Table t, Name id, int size)
			:i(new Internals{t,id,size,ss,nullptr}){
			i->buf1 = (char*) malloc(size);
		}

//existing object
		SQLStore_impl::GSQLObject::GSQLObject(SQLStore_impl& ss, Table t, Name id)
			:i{new Internals{t,id,
					(t == Table::IntStore ? (int) sizeof(int) : -1),
					ss,
					(t == Table::IntStore ? (char*) malloc(sizeof(int)) : nullptr)
					}} {}

//"named" object
		SQLStore_impl::GSQLObject::GSQLObject(SQLTransaction* trans, SQLStore_impl& ss, Table t, Name id, const vector<char> &c)
			:i{new Internals{t,id,(int)c.size(),ss,
					(char*) malloc(c.size())}}{

			{
				//assert(!ro_isValid(trans));

				if (t == Table::BlobStore){
					binarystring blob(&c.at(0),c.size());
					cmds::initialize_with_id(ss.level,*trans,t,SQLConnection::repl_group,id,ss.clock,blob);
				}
				else if (t == Table::IntStore){
					cmds::initialize_with_id(ss.level,*trans,t,SQLConnection::repl_group,id,ss.clock,((int*)c.data())[0]);
				}
                                if (i->_store.level == Level::causal){
                                    for (auto& val : i->causal_vers)
                                        val = 0;
                                }
                                else if (i->_store.level == Level::strong){
                                    i->vers = 0;
                                }
			}
			memcpy(this->i->buf1, &c.at(0), c.size());
		}

		SQLStore_impl& SQLStore_impl::GSQLObject::store() {
			return i->_store;
		}
		
		SQLStore_impl::GSQLObject::~GSQLObject(){
			if (i){
				if (i->buf1) free(i->buf1);
				delete i;
			}
		}

				int SQLStore_impl::GSQLObject::store_instance_id() const {
			return i->store_id;
		}

		Name SQLStore_impl::GSQLObject::name() const {
			return this->i->key;
		}

		const std::array<long long,NUM_CAUSAL_GROUPS>& SQLStore_impl::GSQLObject::timestamp() const {
			return this->i->causal_vers;
		}

		void SQLStore_impl::GSQLObject::save(SQLTransaction *trans){
			assert(trans);
			char *c = i->buf1;
			assert(c);

#define upd_23425(x...) cmds::update_data(i->_store.level,*trans,i->table,SQLConnection::repl_group,i->key,i->_store.clock,x)
	
			if (i->table == Table::BlobStore){
				binarystring blob(c,i->size);
                                auto res = upd_23425(blob);
                                if (i->_store.level == Level::strong){
                                    process_version_update(res,i->vers);
                                }
                                else if (i->_store.level == Level::causal){
                                    process_version_update(res,i->causal_vers);
                                }
			}
			else if (i->table == Table::IntStore){
                                auto res = upd_23425(((int*)c)[0]);
                                if (i->_store.level == Level::strong){
                                    process_version_update(res,i->vers);
                                }
                                else if (i->_store.level == Level::causal){
                                    process_version_update(res,i->causal_vers);
                                }
			}
		}

		char* SQLStore_impl::GSQLObject::load(SQLTransaction *trans){
			assert(trans);
			if (i->size >= max_ver_check_size){
				bool store_same = false;
				if (i->_store.level == Level::causal){
					auto old = i->causal_vers;
					process_version_update(cmds::select_version(i->_store.level, *trans,i->table,i->key),i->causal_vers);
					store_same = ends::is_same(old,i->causal_vers);
					if (!store_same) i->_store.clock = max(i->_store.clock,i->causal_vers);
					assert(i->_store.clock[2] < 30);
				}
				else if (i->_store.level == Level::strong){
					auto old = i->vers;
					int newi = -12;
                                        process_version_update(cmds::select_version(i->_store.level, *trans,i->table,i->key),newi);
					store_same = (old == newi);
					i->vers = newi;
				}
				if (store_same) return i->buf1;
			}
			{
				result r = cmds::select_version_data(i->_store.level,*trans,i->table,i->key);
				int start_offset = 0;
				if (i->_store.level == Level::causal){
					start_offset = process_version_update(r,i->causal_vers);
				}
				else start_offset = process_version_update(r,i->vers);
				if (i->table == Table::BlobStore){
					binarystring bs(r[0][start_offset]);
					i->size = bs.size();
					assert(i->size >= 1);
					if (!i->buf1) i->buf1 = (char*) malloc(i->size);
					memcpy(i->buf1,bs.data(),i->size);
				}
				else if (i->table == Table::IntStore) {
					int res = -1;
                                        if (!r[0][start_offset].to(res)){
						std::cerr << "Attempting to access key "<< i->key << " from IntStore in " <<  i->_store.level << " land" << std::endl;
						assert(false && "no result!");
					}
					((int*)i->buf1)[0] = res;
				}
				assert(i->buf1);
				return i->buf1;
			}
		}

		void SQLStore_impl::GSQLObject::increment(SQLTransaction *trans){
			assert(trans);
			auto r = cmds::increment(i->_store.level,
															 *trans,
															 i->table,
															 SQLConnection::repl_group,
															 i->key,
															 i->_store.clock);
			if (i->_store.level == Level::causal){
				process_version_update(r,i->causal_vers);
			}
			else process_version_update(r,i->vers);
		}

		bool SQLStore_impl::GSQLObject::ro_isValid(SQLTransaction *gso) const {
			assert(gso);
			return obj_exists(i->key,gso);
		}

		void SQLStore_impl::GSQLObject::resize_buffer(std::size_t newsize){
			if(!i->buf1) {
				i->buf1 = (char*) malloc(newsize);
				i->size = newsize;
			}
			else assert(newsize == i->size);
		}

		char* SQLStore_impl::GSQLObject::obj_buffer() {
			assert(i->buf1);
			return i->buf1;
		}

		char const * SQLStore_impl::GSQLObject::obj_buffer() const {
			assert(i->buf1);
			return i->buf1;
		}

		int SQLStore_impl::GSQLObject::obj_buffer_size() const {
			assert(i->size >= 0);
			return i->size;
		}

		int SQLStore_impl::GSQLObject::bytes_size() const {
			return sizeof(int)*4 + sizeof(Level) + sizeof(Table);
		}

		int SQLStore_impl::GSQLObject::to_bytes(char* c) const {
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

		void SQLStore_impl::GSQLObject::post_object(const std::function<void (char const * const,std::size_t)>&f) const {
			auto size = bytes_size();
			char buf[size];
			to_bytes(buf);
			f(buf,size);
		}
		
		SQLStore_impl::GSQLObject SQLStore_impl::GSQLObject::from_bytes(SQLInstanceManager_abs& mgr, char const *_v){
			//arr[0] has already been used to find this implementation
			auto *v = _v + sizeof(int);
			int* arr = (int*)v;
			Level* arrl = (Level*) (arr + 3);
			Table* arrt = (Table*) (arrl + 1);
			//of from_bytes
			Level lvl = arrl[0];
			return GSQLObject(mgr.inst(lvl),
							  arrt[0],arr[0],arr[1]);
		}
	}
}
