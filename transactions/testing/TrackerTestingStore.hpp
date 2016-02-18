#pragma once
#include "Tracker.hpp"
#include <map>

namespace myria { namespace testing {
		template<Level l>
		class TrackerTestingStore : public DataStore<l>{
		public:
			TrackerTestingStore(tracker::Tracker& t){
				t.registerStore(*this);
			}

			TrackerTestingStore(const TrackerTestingStore&) = delete;

			class AlwaysSuccessfulTransaction : public mtl::StoreContext<l> {
			public:
				TrackerTestingStore &tts;

				AlwaysSuccessfulTransaction(TrackerTestingStore &tts)
					:tts(tts){}
				
				DataStore<l> &store(){
					return tts;
				}

				bool store_commit(){
					return true;
				}
			};
			
			std::unique_ptr<mtl::StoreContext<l> > begin_transaction(){
				return std::unique_ptr<mtl::StoreContext<l> >(
					new AlwaysSuccessfulTransaction{*this});
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
			class TrackerTestingObject : public RemoteObject<l,T> {

				TrackerTestingStore &tts;
				const Name nam;
				std::shared_ptr<const T> t;
				const std::array<int,4> causal_vers{{5,5,5,5}};
			public:

				TrackerTestingObject(TrackerTestingStore& tts, Name nam)
					:tts(tts),nam(nam),t(new T{*remote_store().rs.template at<T>(nam)}){}
				
				TrackerTestingObject(TrackerTestingStore& tts, Name nam, T t)
					:tts(tts),nam(nam),t(new T{t}){
					if (l == Level::strong || nam < 10)
						remote_store().rs.template mut<T>(nam).reset(new T{t});
				}

				TrackerTestingObject(const TrackerTestingObject& tto)
					:tts(tto.tts),nam(tto.nam),t(tto.t),causal_vers(tto.causal_vers)
					{
					}

				bool ro_isValid(mtl::StoreContext<l>*) const {
					return true;
				}
				
				const T& get(mtl::StoreContext<l>*, tracker::Tracker*/* = nullptr*/, tracker::TrackingContext*/* = nullptr*/) {
					if (remote_store().rs.contains(nam))
						this->t = std::make_shared<T>(*remote_store().rs.template at<T>(nam));
					return *t;
				}
				
				void put(mtl::StoreContext<l>*,const T& to) {
					this->t = std::make_shared<T>(to);
					if (l == Level::strong)
						remote_store().rs.template mut<T>(nam).reset(new T(*t));
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
					((TrackerTestingObject**)(iarr + 1))[0] = new TrackerTestingObject(*this);
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
			auto newObject(tracker::Tracker &trk, mtl::TransactionContext *tc, Name name, const T& init){
				auto ret = make_handle<l,ha,T,TrackerTestingObject<T> >
					(trk,tc,*this,name,init);
				trk.onCreate(*this,name);
				return ret;
			}

			template<HandleAccess ha, typename T>
			auto existingObject(tracker::Tracker &trk, mtl::TransactionContext *tc, Name name, T* for_inf = nullptr){
				return make_handle
					<l,ha,T,TrackerTestingObject<T> >
					(trk,tc,*this,name);
			}

			template<typename T>
			std::unique_ptr<TrackerTestingObject<T> > existingRaw(Name name, T* for_inf = nullptr){
				return std::unique_ptr<TrackerTestingObject<T> >
				{new TrackerTestingObject<T>{*this,name}};
			}

			bool exists(Name name){
				return remote_store().rs.contains(name);
			}

		};
	}}
