#pragma once
#include "Tracker.hpp"
#include <map>

namespace myria { namespace testing {

		using mutils::ReassignableReference;
		using mutils::abs_StructBuilder;
		
		enum class TrackerTestingMode {
			perfect, manual_sync
		};
		
		template<Level l, TrackerTestingMode mode = TrackerTestingMode::perfect>
		class TrackerTestingStore : public DataStore<l>{
		public:

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
				abs_StructBuilder &logger;

				AlwaysSuccessfulTransaction(TrackerTestingStore &tts,abs_StructBuilder& logger)
					:tts(tts),logger(logger){
					logger.incrementIntField(
						LogFields::trackertesting_transaction_built);
				}
				
				DataStore<l> &store(){
					return tts;
				}

				bool store_commit(){
					logger.incrementIntField(
						LogFields::trackertesting_transaction_commit);
					return true;
				}

				void store_abort(){
					logger.incrementIntField(
						LogFields::trackertesting_transaction_abort);
				}
			};
			
			std::unique_ptr<mtl::StoreContext<l> > begin_transaction(mutils::abs_StructBuilder& abs){
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
					((AlwaysSuccessfulTransaction*)tc)->logger.incrementIntField(LogFields::trackertestingobject_isvalid);

					return true;
				}
				
				std::shared_ptr<const T> get(mtl::StoreContext<l>* tc, tracker::Tracker* trk/* = nullptr*/,
							 tracker::TrackingContext* trkc/* = nullptr*/) {

					assert(tc);
					((AlwaysSuccessfulTransaction*)tc)->logger.incrementIntField(LogFields::trackertestingobject_get);
					
					if (remote_store().rs.contains(nam))
						this->t = std::make_unique<T>(*remote_store().rs.template at<T>(nam));
					assert(trkc);
					this->t = trk->onRead(*trkc,store(),name(),timestamp(),std::move(t),(T*)nullptr);
					return std::make_shared<T>(*t);
				}
				
				void put(mtl::StoreContext<l>* tc,const T& to) {
					assert(tc);
					((AlwaysSuccessfulTransaction*)tc)->logger.incrementIntField(LogFields::trackertestingobject_put);
					
					this->t = std::make_unique<T>(to);
					if (l == Level::strong)
						remote_store_set(nam,to);
					else {
						tts.causal_remote_propogation(nam,to);
					}
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
			
			template<typename T, Level l2>
			static TrackerTestingObject<T>* tryCast(RemoteObject<l2,T>* r) {
				if(auto *ret = dynamic_cast<TrackerTestingObject<T>* >(r)){
					assert(l2 == l);
					return ret;
				}
				else throw mtl::ClassCastException();
			}
	
			template<typename T, restrict(!(is_RemoteObj_ptr<T>::value))>
			static auto tryCast(T && r){
				return std::forward<T>(r);
			}


			template<typename T>
			static std::unique_ptr<TrackerTestingObject<T> > from_bytes(char const * v){
				TrackerTestingObject<T> *ptr = ((TrackerTestingObject<T>**) v)[0];
				auto ret = std::make_unique<TrackerTestingObject<T> >(*ptr);
				delete ptr;
				return ret;
			}

			
			template<HandleAccess ha, typename T>
			auto newObject(tracker::Tracker &trk, mtl::TransactionContext *tc, Name name, const T& init){
				assert(tc);
				((AlwaysSuccessfulTransaction*)tc)->logger.incrementIntField(LogFields::trackertesting_newobject);
				auto ret = make_handle<l,ha,T,TrackerTestingObject<T> >
					(trk,tc,*this,name,init);
				trk.onCreate(*this,name, (T*)nullptr);
				return ret;
			}

			template<HandleAccess ha, typename T>
			auto existingObject(tracker::Tracker &trk, mtl::TransactionContext *tc, Name name, T* for_inf = nullptr){
				assert(tc);
				((AlwaysSuccessfulTransaction*)tc)->logger.incrementIntField(
					LogFields::trackertesting_existingobject);
				return make_handle
					<l,ha,T,TrackerTestingObject<T> >
					(trk,tc,*this,name);
			}

			template<typename T>
			std::unique_ptr<TrackerTestingObject<T> > existingRaw(abs_StructBuilder& logger, Name name, T* for_inf = nullptr){
				logger.incrementIntField(LogFields::trackertesting_existingraw);
				return std::unique_ptr<TrackerTestingObject<T> >
				{new TrackerTestingObject<T>{*this,name}};
			}

			bool exists(Name name){
				return remote_store().rs.contains(name);
			}

			OPERATION(Increment, TrackerTestingObject<int>* o) {
				//TODO: ideally this would be automatically reduced to the correct type,
				//but for now we'll just manually do it here.
				assert(transaction_context && "Error: calling operations outside of transactions is disallowed");
				AlwaysSuccessfulTransaction *ctx = (l == Level::strong ?
								   dynamic_cast<AlwaysSuccessfulTransaction*>(transaction_context->strongContext.get()) :
								   dynamic_cast<AlwaysSuccessfulTransaction*>(transaction_context->causalContext.get()));
				assert(ctx && "error: should have entered transaction before this point!");
				transaction_context->logger->incrementIntField(LogFields::trackertesting_increment);
				o->put(ctx,*o->t + 1);
				return true;
			}
			END_OPERATION

		};
	}}
