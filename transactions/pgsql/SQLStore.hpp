#pragma once
#include <iostream>
#include "SQLStore_impl.hpp"
#include "Operations.hpp"
#include "SQLTransaction.hpp"

namespace myria { namespace pgsql {

		template<Level l>
		using level_to_label = std::conditional_t<l == Level::causal, Label<causal>, Label<strong> >;
		
		template<Level l>
		using choose_strong = std::integral_constant<bool, l == Level::strong>*;
		template<Level l>
		using choose_causal = std::integral_constant<bool, l == Level::causal>*;
		
		template<Level l>
		class SQLStore : public SQLStore_impl, public TrackableDataStore<SQLStore<l>,  level_to_label<l> > {
		public:

			using SQLStore_impl::exists;
			using TrackableDataStore<SQLStore<l>,  level_to_label<l> >::exists;
			
			static constexpr Level level = l;
			using label = level_to_label<l>;
			
			virtual ~SQLStore() {}

			struct SQLInstanceManager : public SQLInstanceManager_abs{
			public:
				whenpool(SQLConnectionPool<l> &p;)
				whennopool(std::string p;)
				SQLInstanceManager(decltype(p) p)
					:SQLInstanceManager_abs(),p(p){
				}
				SQLInstanceManager(const SQLInstanceManager&) = delete;
				virtual ~SQLInstanceManager(){
					for (auto &p : ss){
						SQLStore* ptr = p.second.release();
						delete ptr;
					}
				}
			private:
				std::map<int,std::unique_ptr<SQLStore> > ss;
				
				void inst(Level whendebug(l2)){
					assert(l == l2);
					if (ss.count(0) == 0 || (!ss.at(0))){
						assert(this->this_mgr);
						ss[0].reset(new SQLStore(*this->this_mgr,p));
					}
				}

				SQLStore<Level::strong>& choose_s(std::true_type*){
					assert(ss.at(0));
					return *ss.at(0);
				}
				
				SQLStore<Level::strong>& choose_s(std::false_type*){
					assert(false && "Error: This is not a strong instance manager");
					struct die{}; throw die{};
				}
				
				SQLStore<Level::causal>& choose_c(std::true_type*){
					assert(ss.at(0));
					return *ss.at(0);
				}
				
				SQLStore<Level::causal>& choose_c(std::false_type*){
					assert(false && "Error: This is not a causal instance manager");
					struct die{}; throw die{};
				}
				
			public:
				
				SQLStore<Level::strong>& inst_strong(){
					inst(Level::strong);
					choose_strong<l> choice{nullptr};
					return choose_s(choice);
				}
				
				SQLStore<Level::causal>& inst_causal(){
					inst(Level::causal);
					choose_causal<l> choice{nullptr};
					return choose_c(choice);
				}

				auto& inst(){
					inst(l);
					assert(ss.at(0));
					return *ss.at(0);
				}
			};

			mutils::DeserializationManager &this_mgr;

		private:
			SQLStore(mutils::DeserializationManager &this_mgr,whenpool(GeneralSQLConnectionPool) whennopool(const std::string) &p)
			  :GDataStore(label::description),SQLStore_impl(p,*this,l),TrackableDataStore<SQLStore,label>(),this_mgr(this_mgr) {
			}
		public:

			using deserialization_context = SQLInstanceManager;

			using Store = SQLStore;	

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
#ifndef NDEBUG
			std::string why_in_transaction() const {
#ifndef NOPOOL
				if (in_transaction()){
					assert(this->default_connection.acquire_if_locked()->current_trans);
					return this->default_connection.acquire_if_locked()->current_trans->why;
				}
				else return "error: not in transaction";
#else
				return "no idea, sorry";
#endif
			}
#endif

			const std::array<unsigned long long, NUM_CAUSAL_GROUPS>& local_time() const {
				return this->clock;
			}

			template<typename T>
			struct SQLObject : public RemoteObject<label,T> {
				using Store = SQLStore;
				GSQLObject gso;
				std::shared_ptr<T> t;
				mutils::DeserializationManager &tds;

				SQLObject(GSQLObject gs, std::unique_ptr<T> _t, mutils::DeserializationManager &tds):
					gso(std::move(gs)),tds(tds){
					assert(this);
					if (_t){
#ifndef NDEBUG
						mutils::ensure_registered(*_t,tds);
#endif
						this->t = std::shared_ptr<T>{_t.release()};
					}
				}
				int fail_counter = 0;

		
				std::shared_ptr<const T> get(mtl::StoreContext<label>* _tc) {
					SQLContext *sctx = (SQLContext*) _tc;
					SQLTransaction *tc = (_tc ? sctx->i.get() : nullptr);
					auto *res = gso.load(tc);
					assert(res);
					t.reset(mutils::from_bytes<T>(&tds,res).release());
					return t;
				}

				const std::array<unsigned long long,NUM_CAUSAL_GROUPS>& timestamp() const {
					return gso.timestamp();
				}

				void put(mtl::StoreContext<label>* _tc, const T& t){
					SQLTransaction *tc = (_tc ? ((SQLContext*) _tc)->i.get() : nullptr);
					this->t = std::make_shared<T>(t);
					gso.resize_buffer(mutils::bytes_size(t));
					mutils::to_bytes(t,gso.obj_buffer());
					gso.save(tc);
				}

				bool ro_isValid(mtl::StoreContext<label>* _tc) const{
					SQLTransaction *tc = (_tc ? ((SQLContext*) _tc)->i.get() : nullptr);
					return gso.ro_isValid(tc);
				}
				const SQLStore& store() const{
					return tds.template mgr<SQLInstanceManager>().inst();
				}
				SQLStore& store(){
					return tds.template mgr<SQLInstanceManager>().inst();
				}
				Name name() const {
					return gso.name();
				}
				std::size_t bytes_size() const {
					return gso.bytes_size();
				}
				std::size_t to_bytes(char* c) const {
					return gso.to_bytes(c);
				}
				void post_object(const std::function<void (char const * const,std::size_t)>&f) const {
					return gso.post_object(f);
				}
#ifndef NDEBUG
				void ensure_registered(mutils::DeserializationManager &m){
					assert(m. template registered<deserialization_context>());
				}
#endif
			};

			template<typename T>
			using SQLHandle =
				std::conditional_t<mutils::is_set<T>::value,
													 Handle<label,T,SupportedOperation<RegisteredOperations::Insert,void,SelfType,mutils::extract_type_if_set<T> > >,
													 std::conditional_t<
														 std::is_same<T,int>::value,
														 Handle<label,T,SupportedOperation<RegisteredOperations::Increment,void,SelfType> >,
														 Handle<label,T> > >;

		  struct SQLContext;
			template<typename T>
			SQLHandle<T> newObject(SQLContext *ctx, Name name, const T& init){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				int size = mutils::bytes_size(init);
				std::vector<char> v(size);
				whendebug(int tb_size = mutils::to_bytes(init,&v[0]));
				assert(size == tb_size);
				GSQLObject gso(ctx->i.get(),*this,t,name,v);
				SQLHandle<T> ret{std::make_shared<SQLObject<T> >(std::move(gso),mutils::heap_copy(init),this_mgr),*this };
				return ret;
			}

			template<typename T>
			auto newObject(SQLContext *, const T& init){
				return newObject<T>(mutils::int_rand(),init);
			}

			bool exists(SQLContext* ctx, Name n){
				return exists((ctx ? ctx->i.get() : nullptr),n);
			}

			template<typename T>
			auto existingObject(SQLContext *, Name name){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				GSQLObject gso(*this,t,name);
				return SQLHandle<T>{std::make_shared<SQLObject<T> >(std::move(gso),nullptr,this_mgr),*this};
			}

			//deserializing this RemoteObject, not its stored thing.
			template<typename T>
			static std::unique_ptr<SQLHandle<T> > from_bytes(mutils::DeserializationManager* mngr, char const * v){
				//this really can't be called via the normal deserialization process; it needs to be called in the process of deserializing a RemoteObject.
				assert(mngr);
				auto &insance_manager = mngr->template mgr<deserialization_context>();
				auto gsql_obj = GSQLObject::from_bytes(insance_manager,v);
				auto &this_ds = dynamic_cast<SQLStore&>(gsql_obj.store()); //this should never fail
				return std::make_unique<SQLHandle<T> >(std::make_shared<SQLObject<T> >(std::move(gsql_obj),nullptr,*mngr),this_ds);
			}

			struct SQLContext : public mtl::StoreContext<label> {
				std::unique_ptr<SQLTransaction> i;
				mutils::DeserializationManager & mngr;
				SQLContext(decltype(i) i, mutils::DeserializationManager& mngr):i(std::move(i)),mngr(mngr){}
				DataStore<label>& store() {return dynamic_cast<DataStore<label>&>( i->gstore);}
				bool store_commit() {return i->store_commit();}
			  bool store_abort() {i->store_abort(); return true;}
			};

			using StoreContext = SQLContext;

			std::unique_ptr<mtl::StoreContext<label> > begin_transaction(whendebug(const std::string &why))
				{
					auto ret = SQLStore_impl::begin_transaction(whendebug(why));
					return std::unique_ptr<mtl::StoreContext<label> >(new SQLContext{std::move(ret),this_mgr});
				}

			bool in_transaction() const {
				return SQLStore_impl::in_transaction();
			}

			int instance_id() const {
				return SQLStore_impl::instance_id();
			}

			void operation(mtl::PhaseContext<label>*, SQLContext& ctx,
						   OperationIdentifier<RegisteredOperations::Increment>, SQLObject<int> &o){
				o.gso.increment(ctx.i.get());
			}

			template<typename T>
			SQLHandle<T> operation(mtl::PhaseContext<label>* transaction_context, SQLContext& ctx,
													 OperationIdentifier<RegisteredOperations::Clone>, SQLObject<T> &o){
				return newObject<T>(transaction_context,*o.get(&ctx));
			}

			template<typename T>
			void operation(mtl::PhaseContext<label>*, SQLContext& ,
						   OperationIdentifier<RegisteredOperations::Insert>, SQLObject<std::set<T> > &, T& ){
				//assert(false && "this is unimplemented.");
				//o.gso.increment(ctx->i.get());
			}
		};
	}}
#include "trackable_datastore_impl.hpp"
