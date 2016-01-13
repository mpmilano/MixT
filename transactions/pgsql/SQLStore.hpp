#pragma once
#include "SQLStore_impl.hpp"
#include "Tracker_common.hpp"
#include "Tracker_support_structs.hpp"
#include "SQLTransaction.hpp"

namespace myria { namespace pgsql {
	
		template<Level l>
		class SQLStore : public SQLStore_impl, public DataStore<l> {

			SQLStore(int inst_id):SQLStore_impl(*this,inst_id,l) {
				tracker::Tracker::global_tracker().registerStore(*this);
			}
		public:

			using Store = SQLStore;
	
			static SQLStore& inst(int instance_id){
				static std::map<int,SQLStore* > ss;
				if (ss.count(instance_id) == 0){
					ss[instance_id] = new SQLStore(instance_id);
				}
				return *ss.at(instance_id);
			}

			static constexpr int id() {
				return SQLStore_impl::ds_id_nl() + (int) l;
			}

			int ds_id() const {
				assert(SQLStore_impl::ds_id() == id());
				return id();
			}

			SQLStore& store() {
				return *this;
			}

			const std::array<int, NUM_CAUSAL_GROUPS>& local_time() const {
				return this->clock;
			}

			template<typename T>
			struct SQLObject : public RemoteObject<l,T> {
				using Store = SQLStore;
				GSQLObject gso;
				std::unique_ptr<T> t;

				SQLObject(GSQLObject gs, std::unique_ptr<T> t):
					gso(std::move(gs)),t(std::move(t)){}
		
				const T& get(mtl::TransactionContext* tc) {
					char * res = nullptr;
					res = gso.load(tc);
					assert(res);
					if (res != nullptr){
						auto &trk = tracker::Tracker::global_tracker();
						t = trk.onRead(store(),name(),timestamp(),
									   mutils::from_bytes<T>(res));
					}
					
					return *t;
				}

				const std::array<int,NUM_CAUSAL_GROUPS>& timestamp() const {
					return gso.timestamp();
				}

				std::vector<char> bytes() const {
					std::vector<char> ret(gso.obj_buffer_size());
					memcpy(ret.data(),gso.obj_buffer(),gso.obj_buffer_size());
					return ret;
				}

				void put(mtl::TransactionContext* tc, const T& t){
					this->t = std::make_unique<T>(t);
					mutils::to_bytes(t,gso.obj_buffer());
					gso.save(tc);
				}

				//these just forward
				void setTransactionContext(mtl::TransactionContext* t){
					gso.setTransactionContext(t);
				}
				mtl::TransactionContext* currentTransactionContext(){
					return gso.currentTransactionContext();
				}
				bool ro_isValid(mtl::TransactionContext* tc) const{
					return gso.ro_isValid(tc);
				}
				const SQLStore& store() const{
					return SQLStore::inst(gso.store_instance_id());
				}
				SQLStore& store(){
					return SQLStore::inst(gso.store_instance_id());
				}
				Name name() const {
					return gso.name();
				}
				int bytes_size() const {
					return gso.bytes_size();
				}
				int to_bytes(char* c) const {
					return gso.to_bytes(c);
				}
			};

			template<typename T, Level l2>
			static SQLObject<T>* tryCast(RemoteObject<l2,T>* r) {
				if(auto *ret = dynamic_cast<SQLObject<T>* >(r)){
					assert(l2 == l);
					return ret;
				}
				else throw mtl::Transaction::ClassCastException();
			}
	
			template<typename T, restrict(!(is_RemoteObj_ptr<T>::value))>
			static auto tryCast(T && r){
				return std::forward<T>(r);
			}
	
			template<HandleAccess ha, typename T>
			auto newObject(Name name, const T& init){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				int size = mutils::bytes_size(init);
				std::vector<char> v(size);
				assert(size == mutils::to_bytes(init,&v[0]));
				GSQLObject gso(*this,t,name,v);
				auto ret = make_handle
					<l,ha,T,SQLObject<T> >
					(std::move(gso),mutils::heap_copy(init) );
				ret.tracker.onCreate(*this,name);
				return ret;
			}

			template<HandleAccess ha, typename T>
			auto newObject(const T& init){
				return newObject<ha,T>(mutils::int_rand(),init);
			}

			template<HandleAccess ha, typename T>
			auto existingObject(Name name, T* for_inf = nullptr){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				GSQLObject gso(*this,t,name);
				return make_handle
					<l,ha,T,SQLObject<T> >
					(std::move(gso),nullptr);
			}

			template<typename T>
			std::unique_ptr<SQLObject<T> > existingRaw(Name name, T* for_inf = nullptr){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				return std::unique_ptr<SQLObject<T> >
				{new SQLObject<T>{GSQLObject{*this,t,name},nullptr}};
			}

			template<typename T>
			static std::unique_ptr<SQLObject<T> > from_bytes(char const * v){
				return std::make_unique<SQLObject<T> >(GSQLObject::from_bytes(v),
													   std::unique_ptr<T>());
			}

			std::unique_ptr<mtl::StoreContext<l> > begin_transaction()
				{
					struct StoreContext : mtl::StoreContext<l> {
						std::unique_ptr<SQLTransaction> i;
						StoreContext(decltype(i) i):i(std::move(i)){}
						DataStore<l>& store() {return dynamic_cast<DataStore<l>&>( i->gstore);}
						bool store_commit() {return i->store_commit();}
					};
					auto ret = SQLStore_impl::begin_transaction();
					return std::unique_ptr<mtl::StoreContext<l> >(new StoreContext{std::move(ret)});
				}

			int instance_id() const {
				return SQLStore_impl::instance_id();
			}

			OPERATION(Increment, mtl::TransactionContext* transaction_context, SQLObject<int>* o) {
				o->gso.increment(transaction_context);
				return true;
			}
			END_OPERATION
		};

	}}
