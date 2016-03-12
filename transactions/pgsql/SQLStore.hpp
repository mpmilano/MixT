#pragma once
#include "SQLStore_impl.hpp"
#include "Tracker_common.hpp"
#include "Tracker_support_structs.hpp"
#include "SQLTransaction.hpp"

namespace myria { namespace pgsql {
	
		template<Level l>
		class SQLStore : public SQLStore_impl, public DataStore<l> {
		public:

			
			struct SQLInstanceManager : public SQLInstanceManager_abs{
			public:
                                tracker::Tracker &trk;
                                SQLInstanceManager(tracker::Tracker &trk):trk(trk){
									std::cout << "new SQLInstance manager created" << std::endl;
								}
                                SQLInstanceManager(const SQLInstanceManager&) = delete;
                                virtual ~SQLInstanceManager(){}
			private:
				std::map<int,std::unique_ptr<SQLStore> > ss;
				
				void inst(Level l2, int instance_id){
					assert(l == l2);
					if (ss.count(instance_id) == 0){
						assert(this->this_mgr);
						ss[instance_id].reset(new SQLStore(trk,instance_id,*this->this_mgr));
					}
				}

				SQLStore<Level::strong>& choose_s(int instance_id, std::true_type*){
					return *ss.at(instance_id);
				}
				
				SQLStore<Level::strong>& choose_s(int instance_id, std::false_type*){
					assert(false && "Error: This is not a strong instance manager");
				}
				
				SQLStore<Level::causal>& choose_c(int instance_id, std::true_type*){
					return *ss.at(instance_id);
				}
				
				SQLStore<Level::causal>& choose_c(int instance_id, std::false_type*){
					assert(false && "Error: This is not a causal instance manager");
				}
				
			public:
				
				SQLStore<Level::strong>& inst_strong(int instance_id){
					inst(Level::strong,instance_id);
					choose_strong<l> choice{nullptr};
					return choose_s(instance_id, choice);
				}
				
				SQLStore<Level::causal>& inst_causal(int instance_id){
					inst(Level::causal,instance_id);
					choose_causal<l> choice{nullptr};
					return choose_c(instance_id, choice);
				}

				auto& inst(int instance_id){
					inst(l,instance_id);
					return *ss.at(instance_id);
				}
			};

			mutils::DeserializationManager &this_mgr;

		private:
			SQLStore(tracker::Tracker& trk, int inst_id, mutils::DeserializationManager &this_mgr):SQLStore_impl(*this,inst_id,l),this_mgr(this_mgr) {
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

			bool in_transaction() const {
				bool it = this->default_connection->in_trans();
				assert ([&](){
						bool ct = this->default_connection->current_trans;
						return (it ? ct : true);}());
				return it;
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

				/*
				std::vector<char> bytes() const {
					std::vector<char> ret(gso.obj_buffer_size());
					memcpy(ret.data(),gso.obj_buffer(),gso.obj_buffer_size());
					return ret;
				}//*/

				void put(mtl::StoreContext<l>* _tc, const T& t){
					SQLTransaction *tc = (_tc ? ((SQLContext*) _tc)->i.get() : nullptr);
					this->t = std::make_shared<T>(t);
					mutils::to_bytes(t,gso.obj_buffer());
					gso.save(tc);
				}

				bool ro_isValid(mtl::StoreContext<l>* _tc) const{
					SQLTransaction *tc = (_tc ? ((SQLContext*) _tc)->i.get() : nullptr);
					return gso.ro_isValid(tc);
				}
				const SQLStore& store() const{
					return tds.template mgr<SQLInstanceManager>().inst(gso.store_instance_id());
				}
				SQLStore& store(){
					return tds.template mgr<SQLInstanceManager>().inst(gso.store_instance_id());
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
				void ensure_registered(mutils::DeserializationManager &m){
					assert(m. template registered<deserialization_context>());
				}
			};

			template<typename T, Level l2>
			static SQLObject<T>* tryCast(RemoteObject<l2,T>* r) {
				if(auto *ret = dynamic_cast<SQLObject<T>* >(r)){
					assert(l2 == l);
					return ret;
				}
				else throw mtl::ClassCastException();
			}
	
			template<typename T, restrict(!(is_RemoteObj_ptr<T>::value))>
			static auto tryCast(T && r){
				return std::forward<T>(r);
			}
	
			template<HandleAccess ha, typename T>
			auto newObject(tracker::Tracker &trk, mtl::TransactionContext *tc, Name name, const T& init){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				int size = mutils::bytes_size(init);
				std::vector<char> v(size);
				assert(size == mutils::to_bytes(init,&v[0]));
				GSQLObject gso(*this,t,name,v);
				auto ret = make_handle
					<l,ha,T,SQLObject<T> >
					(trk,tc,std::move(gso),mutils::heap_copy(init),this_mgr);
				trk.onCreate(*this,name,(T*)nullptr);
				return ret;
			}

			template<HandleAccess ha, typename T>
			auto newObject(tracker::Tracker &trk, mtl::TransactionContext *tc, const T& init){
				return newObject<ha,T>(trk,tc, mutils::int_rand(),init);
			}

			template<HandleAccess ha, typename T>
			auto existingObject(tracker::Tracker &trk, mtl::TransactionContext *tc, Name name, T* for_inf = nullptr){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				GSQLObject gso(*this,t,name);
				return make_handle
					<l,ha,T,SQLObject<T> >
					(trk,tc,std::move(gso),nullptr,this_mgr);
			}

			template<typename T>
			std::unique_ptr<SQLObject<T> > existingRaw(Name name, T* for_inf = nullptr){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				return std::unique_ptr<SQLObject<T> >
				{new SQLObject<T>{GSQLObject{*this,t,name},nullptr,this_mgr}};
			}

			//deserializing this RemoteObject, not its stored thing.
			template<typename T>
			static std::unique_ptr<SQLObject<T> > from_bytes(mutils::DeserializationManager* mngr, char const * v){
                            assert(mngr);
                            return std::make_unique<SQLObject<T> >(GSQLObject::from_bytes(mngr->template mgr<deserialization_context>(),v),
                                                                                                           nullptr,*mngr);
			}

			struct SQLContext : mtl::StoreContext<l> {
				std::unique_ptr<SQLTransaction> i;
				mutils::DeserializationManager & mngr;
				SQLContext(decltype(i) i, mutils::DeserializationManager& mngr):i(std::move(i)),mngr(mngr){}
				DataStore<l>& store() {return dynamic_cast<DataStore<l>&>( i->gstore);}
				bool store_commit() {return i->store_commit();}
				void store_abort() {i->store_abort();}
			};

			std::unique_ptr<mtl::StoreContext<l> > begin_transaction()
				{
					auto ret = SQLStore_impl::begin_transaction();
					return std::unique_ptr<mtl::StoreContext<l> >(new SQLContext{std::move(ret),this_mgr});
				}

			int instance_id() const {
				return SQLStore_impl::instance_id();
			}

			OPERATION(Increment, SQLObject<int>* o) {
				//TODO: ideally this would be automatically reduced to the correct type,
				//but for now we'll just manually do it here.
				assert(transaction_context && "Error: calling operations outside of transactions is disallowed");
				SQLContext *ctx = (l == Level::strong ?
								   dynamic_cast<SQLContext*>(transaction_context->strongContext.get()) :
								   dynamic_cast<SQLContext*>(transaction_context->causalContext.get()));
				assert(ctx && "error: should have entered transaction before this point!");
				o->gso.increment(ctx->i.get());
				return true;
			}
			END_OPERATION
		};

	}}
