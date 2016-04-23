#pragma once
#include "Tracker.hpp"
#include <type_traits>
#include <map>

namespace myria { namespace testing {

		using mutils::ReassignableReference;
		using mutils::abs_StructBuilder;
		
		enum class TrackerTestingMode {
			perfect, manual_sync
		};


		template<Level l, HandleAccess ha, typename T>
		using TestingHandle = std::conditional_t<
			std::is_integral<T>::value,
			Handle<l,ha,T,SupportedOperation<RegisteredOperations::Increment,SelfType> >,
			Handle<l,ha,T> >;
		
		template<Level l, TrackerTestingMode mode = TrackerTestingMode::perfect>
		class TrackerTestingStore : public DataStore<l>{
		public:

			template<HandleAccess ha, typename T>
			using TestingHandle = TestingHandle<l,ha,T>;

			using Store = TrackerTestingStore;
			
			TrackerTestingStore(tracker::Tracker& t)
				:DataStore<l>()
				{
					t.registerStore(*this);
				}

			TrackerTestingStore(const TrackerTestingStore&) = delete;

			class AlwaysSuccessfulTransaction : public mtl::StoreContext<l> {
			public:
				TrackerTestingStore &tts;
                                std::unique_ptr<abs_StructBuilder> &logger;

                                AlwaysSuccessfulTransaction(TrackerTestingStore &tts,std::unique_ptr<abs_StructBuilder>& logger)
					:tts(tts),logger(logger){
                                        logger->incrementIntField(
						LogFields::trackertesting_transaction_built);
					
				}
				
				DataStore<l> &store(){
					return tts;
				}

				bool store_commit(){
                                        logger->incrementIntField(
						LogFields::trackertesting_transaction_commit);
					
					return true;
				}

				void store_abort(){
                                        logger->incrementIntField(
						LogFields::trackertesting_transaction_abort);
					
				}
			};
			
			std::unique_ptr<mtl::StoreContext<l> > begin_transaction(std::unique_ptr<mutils::abs_StructBuilder>& abs, const std::string&){
				return std::unique_ptr<mtl::StoreContext<l> >(
					new AlwaysSuccessfulTransaction{*this,abs});
			}

			const std::array<int, NUM_CAUSAL_GROUPS>& local_time() const {
				static const std::array<int, NUM_CAUSAL_GROUPS> zeros{{0,0,0,0}};
				return zeros;
			}
			int ds_id() const {
				return 30 + (int) l;
			}
			
			int instance_id() const {
				TrackerTestingStore const * const tts = this;
				return ((int*)&tts)[sizeof(tts) - 1];
			}
			
			bool in_transaction() const {
				return false;
			}

			std::string why_in_transaction() const {
				return "I'm not, actually";
			}

			static auto remote_store(){
				static mutils::HeteroMap<Name> rs;
				static std::mutex mut;
				using lock_t = std::unique_lock<std::mutex>;
				struct LockedMapAccess{
					mutils::HeteroMap<Name> &rs;
					lock_t lock;
				};
				return LockedMapAccess{rs,lock_t{mut}};
			}

			template<typename T>
			static void remote_store_set(Name nam, const T &t){
				remote_store().rs.template mut<T>(nam).reset(new T{t});
			}

			std::list<std::function<void ()> > deferred_commits;

			template<typename T>
			void causal_remote_propogation(const Name nam, const T &t){
				//various modes are possible, controlled by class template parameter
				switch(mode){
				case TrackerTestingMode::perfect:
					remote_store_set(nam,t);
					break;
				case TrackerTestingMode::manual_sync:
					deferred_commits.push_back([=](){
							remote_store_set(nam,t);
						});
					break;
				}
			}

			void flush_deferred(){
				for (const auto &f : deferred_commits){
					f();
				}
				deferred_commits.clear();
			}
			
			template<typename T>
			class TrackerTestingObject : public RemoteObject<l,T> {

				TrackerTestingStore &tts;
				const Name nam;
				std::unique_ptr<const T> t;
				const std::array<int,4> causal_vers{{5,5,5,5}};

				TrackerTestingObject(TrackerTestingStore &tts,
									 Name nam,
									 const T& t,
									 const decltype(causal_vers)& cv
					):tts(tts),nam(nam),t(new T(t)),causal_vers(cv){}
				
			public:

				friend class TrackerTestingStore;
				using Store = TrackerTestingStore;

				TrackerTestingObject(TrackerTestingStore& tts, Name nam)
					:tts(tts),nam(nam),t(new T{*remote_store().rs.template at<T>(nam)}){
				}
				
				TrackerTestingObject(TrackerTestingStore& tts, Name nam, T t)
					:tts(tts),nam(nam),t(new T{t}){
					if (l == Level::strong || nam < 10)
						remote_store_set(nam,t);
					else {
						tts.causal_remote_propogation(nam,t);
					}
				}

				void ensure_registered(mutils::DeserializationManager&){
				}

				TrackerTestingObject* clone() const {
					auto &tts2 = const_cast<TrackerTestingStore&>(tts);
					return new TrackerTestingObject(tts2,nam,*t,causal_vers);
				}

				/*
				TrackerTestingObject(const TrackerTestingObject& tto)
					:tts(tto.tts),nam(tto.nam),t(tto.t),causal_vers(tto.causal_vers)
					{
					}
				*/

				bool ro_isValid(mtl::StoreContext<l>* tc) const {
					assert(tc);
                                        ((AlwaysSuccessfulTransaction*)tc)->logger->incrementIntField(LogFields::trackertestingobject_isvalid);
					

					return true;
				}
				
				std::shared_ptr<const T> get(mtl::StoreContext<l>* tc, tracker::Tracker* trk/* = nullptr*/,
							 tracker::TrackingContext* trkc/* = nullptr*/) {

					assert(tc);
                                        ((AlwaysSuccessfulTransaction*)tc)->logger->incrementIntField(LogFields::trackertestingobject_get);
					
					if (remote_store().rs.contains(nam))
						this->t = std::make_unique<T>(*remote_store().rs.template at<T>(nam));
					assert(trkc);
					this->t = trk->onRead(*trkc,store(),name(),timestamp(),std::move(t),(T*)nullptr);
					
					return std::make_shared<T>(*t);
				}
				
                                void raw_put(mtl::StoreContext<l>*,const T& to) {
					this->t = std::make_unique<T>(to);
					if (l == Level::strong)
						remote_store_set(nam,to);
					else {
						tts.causal_remote_propogation(nam,to);
					}
				}

                                void put(mtl::StoreContext<l>* tc,const T& to) {
                                        assert(tc);
                                        ((AlwaysSuccessfulTransaction*)tc)->logger->incrementIntField(LogFields::trackertestingobject_put);
                                        raw_put(tc,to);
                                }
				
				const DataStore<l>& store() const {
					return tts;
				}
				
				DataStore<l>& store() {
					return tts;
				}
				
				Name name() const {
					return nam;
				}

				int bytes_size() const {
					return sizeof(int) + sizeof(TrackerTestingObject*);
				}

				int to_bytes(char *c) const {
					int* iarr = ((int*)c);
					iarr[0] = tts.ds_id();
					((TrackerTestingObject**)(iarr + 1))[0] = clone();
					return bytes_size();
				}
				
				const std::array<int,NUM_CAUSAL_GROUPS>& timestamp() const {
					return causal_vers;
				}

			};

			template<typename T>
			static std::unique_ptr<TrackerTestingObject<T> > from_bytes(char const * v){
				TrackerTestingObject<T> *ptr = ((TrackerTestingObject<T>**) v)[0];
				auto ret = std::make_unique<TrackerTestingObject<T> >(*ptr);
				delete ptr;
				return ret;
			}

			template<HandleAccess ha, typename T>
			TestingHandle<ha,T> newObject(tracker::Tracker &trk, mtl::TransactionContext *tc, Name name, const T& init){
				assert(tc);
                                assert(tc->trackingContext);
                                assert(tc->trackingContext->logger);
                                tc->trackingContext->logger->incrementIntField(LogFields::trackertesting_newobject);
				TestingHandle<ha,T> ret{trk,tc,std::make_shared<TrackerTestingObject<T> >(*this,name,init),*this};
				trk.onCreate(*this,name, (T*)nullptr);
				return ret;
			}

			template<HandleAccess ha, typename T>
                        TestingHandle<ha,T> existingObject(std::unique_ptr<mutils::abs_StructBuilder>& logger, Name name, T* for_inf = nullptr){
                                logger->incrementIntField(
					LogFields::trackertesting_existingobject);
                                return TestingHandle<ha,T>(std::make_shared<TrackerTestingObject<T> >(*this,name),*this);
			}
			
			template<typename T>
			std::unique_ptr<TrackerTestingObject<T> > existingRaw(
                                std::unique_ptr<mutils::abs_StructBuilder>& logger, Name name, T* for_inf = nullptr){
				
                                logger->incrementIntField(LogFields::trackertesting_existingraw);
				return std::unique_ptr<TrackerTestingObject<T> >
				{new TrackerTestingObject<T>{*this,name}};
			}

			bool exists(Name name){
				return remote_store().rs.contains(name);
			}

			void operation(mtl::TransactionContext* _ctx,
						   OperationIdentifier<RegisteredOperations::Increment>, TrackerTestingObject<int> &o){
				AlwaysSuccessfulTransaction *ctx =
					(l == Level::strong ?
					 dynamic_cast<AlwaysSuccessfulTransaction*>(_ctx->strongContext.get()) :
					 dynamic_cast<AlwaysSuccessfulTransaction*>(_ctx->causalContext.get()));
				assert(ctx && "error: should have entered transaction before this point!");
                                ctx->logger->incrementIntField(LogFields::trackertesting_increment);
                                o.raw_put(ctx,*o.t + 1);
			}
		};
	}}
