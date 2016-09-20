#pragma once
#include <iostream>
#include "SQLStore_impl.hpp"
#include "Tracker_common.hpp"
#include "Tracker_support_structs.hpp"
#include "SQLTransaction.hpp"
#include "RemoteCons.hpp"

namespace myria { namespace pgsql {
	
		template<Level l>
		class SQLStore : public SQLStore_impl, public DataStore<l> {
		public:

			static constexpr Level level = l;
			
			virtual ~SQLStore() {}

			struct SQLInstanceManager : public SQLInstanceManager_abs{
			public:
				tracker::Tracker &trk;
				SQLConnectionPool<l> &p;
				SQLInstanceManager(tracker::Tracker &trk, SQLConnectionPool<l> &p)
					:SQLInstanceManager_abs(),trk(trk),p(p){
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
				
				void inst(Level l2){
					assert(l == l2);
					if (ss.count(0) == 0 || (!ss.at(0))){
						assert(this->this_mgr);
						ss[0].reset(new SQLStore(trk,*this->this_mgr,p));
					}
				}

				SQLStore<Level::strong>& choose_s(std::true_type*){
					assert(ss.at(0));
					return *ss.at(0);
				}
				
				SQLStore<Level::strong>& choose_s(std::false_type*){
					assert(false && "Error: This is not a strong instance manager");
				}
				
				SQLStore<Level::causal>& choose_c(std::true_type*){
					assert(ss.at(0));
					return *ss.at(0);
				}
				
				SQLStore<Level::causal>& choose_c(std::false_type*){
					assert(false && "Error: This is not a causal instance manager");
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
			SQLStore(tracker::Tracker& trk, mutils::DeserializationManager &this_mgr,SQLConnectionPool<l> &p)
				:SQLStore_impl(p,*this,l),DataStore<l>(),this_mgr(this_mgr) {
				trk.registerStore(*this);
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

			std::string why_in_transaction() const {
				if (in_transaction()){
					assert(this->default_connection.acquire_if_locked().current_trans());
					return this->default_connection.acquire_if_locked().current_trans()->why;
				}
				else return "error: not in transaction";
			}

			const std::array<int, NUM_CAUSAL_GROUPS>& local_time() const {
				return this->clock;
			}

			template<typename T>
			struct SQLObject : public RemoteObject<l,T> {
				using Store = SQLStore;
				GSQLObject gso;
				std::shared_ptr<T> t;
				mutils::DeserializationManager &tds;

				SQLObject(GSQLObject gs, std::unique_ptr<T> _t, mutils::DeserializationManager &tds):
					gso(std::move(gs)),tds(tds){
					assert(this);
					if (_t){
						mutils::ensure_registered(*_t,tds);
						this->t = std::shared_ptr<T>{_t.release()};
					}
				}
				int fail_counter = 0;

		
				std::shared_ptr<const T> get(mtl::StoreContext<l>* _tc, tracker::Tracker* trk, tracker::TrackingContext* trkc) {
					SQLContext *sctx = (SQLContext*) _tc;
					SQLTransaction *tc = (_tc ? sctx->i.get() : nullptr);
					auto *res = gso.load(tc);
					assert(res);
					if (res != nullptr && trk != nullptr && trkc != nullptr){
						t = trk->onRead(*trkc,store(),name(),timestamp(),
										mutils::from_bytes<T>(&tds,res),(T*)nullptr);
					}
					return t;
				}

				const std::array<int,NUM_CAUSAL_GROUPS>& timestamp() const {
					return gso.timestamp();
				}

				void put(mtl::StoreContext<l>* _tc, const T& t){
					SQLTransaction *tc = (_tc ? ((SQLContext*) _tc)->i.get() : nullptr);
					this->t = std::make_shared<T>(t);
					gso.resize_buffer(mutils::bytes_size(t));
					mutils::to_bytes(t,gso.obj_buffer());
					gso.save(tc);
				}

				bool ro_isValid(mtl::StoreContext<l>* _tc) const{
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
				void ensure_registered(mutils::DeserializationManager &m){
					assert(m. template registered<deserialization_context>());
				}
			};

			template<HandleAccess ha, typename T>
			using SQLHandle = std::conditional_t<is_remote_cons<T>::value,
												 Handle<l,ha,T,SupportedOperation<RegisteredOperations::Clone,SelfType,SelfType> >,
												 std::conditional_t<mutils::is_set<T>::value,
																	Handle<l,ha,T,SupportedOperation<RegisteredOperations::Insert,void,SelfType,mutils::extract_type_if_set<T> > >,
																	std::conditional_t<
																		std::is_same<T,int>::value,
																		Handle<l,ha,T,SupportedOperation<RegisteredOperations::Increment,void,SelfType> >,
																		Handle<l,ha,T> > > >;

			template<HandleAccess ha, typename T>
			SQLHandle<ha,T> newObject(tracker::Tracker &trk, mtl::TransactionContext *tc, Name name, const T& init){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				int size = mutils::bytes_size(init);
				std::vector<char> v(size);
				int tb_size = mutils::to_bytes(init,&v[0]);
				assert(size == tb_size);
				GSQLObject gso(*this,t,name,v);
				SQLHandle<ha,T> ret{trk,tc,std::make_shared<SQLObject<T> >(std::move(gso),mutils::heap_copy(init),this_mgr),*this };
				trk.onCreate(*this,name,(T*)nullptr);
				return ret;
			}

			template<HandleAccess ha, typename T>
			auto newObject(tracker::Tracker &trk, mtl::TransactionContext *tc, const T& init){
				return newObject<ha,T>(trk,tc, mutils::int_rand(),init);
			}

			template<HandleAccess ha, typename T>
			auto existingObject(std::unique_ptr<VMObjectLog>&, Name name){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				GSQLObject gso(*this,t,name);
				return SQLHandle<ha,T>{std::make_shared<SQLObject<T> >(std::move(gso),nullptr,this_mgr),*this};
			}

			template<typename T>
			std::unique_ptr<SQLObject<T> > existingRaw(std::unique_ptr<mutils::abs_StructBuilder>&, Name name){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				return std::unique_ptr<SQLObject<T> >
				{new SQLObject<T>{GSQLObject{*this,t,name},nullptr,this_mgr}};
			}

			//deserializing this RemoteObject, not its stored thing.
			template<HandleAccess ha, typename T>
			static std::unique_ptr<SQLHandle<ha,T> > from_bytes(mutils::DeserializationManager* mngr, char const * v){
				//this really can't be called via the normal deserialization process; it needs to be called in the process of deserializing a RemoteObject.
				assert(mngr);
				auto &insance_manager = mngr->template mgr<deserialization_context>();
				auto gsql_obj = GSQLObject::from_bytes(insance_manager,v);
				auto &this_ds = dynamic_cast<SQLStore&>(gsql_obj.store()); //this should never fail
				return std::make_unique<SQLHandle<ha,T> >(std::make_shared<SQLObject<T> >(std::move(gsql_obj),nullptr,*mngr),this_ds);
			}

			struct SQLContext : mtl::StoreContext<l> {
				std::unique_ptr<SQLTransaction> i;
				mutils::DeserializationManager & mngr;
				SQLContext(decltype(i) i, mutils::DeserializationManager& mngr):i(std::move(i)),mngr(mngr){}
				DataStore<l>& store() {return dynamic_cast<DataStore<l>&>( i->gstore);}
				bool store_commit() {return i->store_commit();}
				void store_abort() {i->store_abort();}
			};

			using StoreContext = SQLContext;

			std::unique_ptr<mtl::StoreContext<l> > begin_transaction(std::unique_ptr<mutils::abs_StructBuilder>&, const std::string &why)
				{
					auto ret = SQLStore_impl::begin_transaction(why);
					return std::unique_ptr<mtl::StoreContext<l> >(new SQLContext{std::move(ret),this_mgr});
				}

			bool in_transaction() const {
				return SQLStore_impl::in_transaction();
			}

			int instance_id() const {
				return SQLStore_impl::instance_id();
			}

			void operation(mtl::TransactionContext*, SQLContext& ctx,
						   OperationIdentifier<RegisteredOperations::Increment>, SQLObject<int> &o){
				o.gso.increment(ctx.i.get());
			}

			template<typename T>
			SQLHandle<HandleAccess::all,T> operation(mtl::TransactionContext* transaction_context, SQLContext& ctx,
													 OperationIdentifier<RegisteredOperations::Clone>, SQLObject<T> &o){
				auto &trk_c = transaction_context->trackingContext;
				auto &trk = trk_c->trk;
				return newObject<HandleAccess::all,T>(trk,transaction_context,*o.get(&ctx,&trk,trk_c.get()));
			}

			template<typename T>
			void operation(mtl::TransactionContext*, SQLContext& ,
						   OperationIdentifier<RegisteredOperations::Insert>, SQLObject<std::set<T> > &, T& ){
				//assert(false && "this is unimplemented.");
				//o.gso.increment(ctx->i.get());
			}
		};
	}}
