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

			std::unique_ptr<mtl::StoreContext<l> > begin_transaction(){
				assert(false && "TODO");
			}

			const std::array<int, NUM_CAUSAL_GROUPS>& local_time() const {
				assert(false && "TODO");
			}
			int ds_id() const {
				return 30 + (int) l;
			}
			
			int instance_id() const {
				return (int) this;
			}
			
			bool in_transaction() const {
				return false;
			}

			template<typename T>
			class TrackerTestingObject : public RemoteObject<l,T> {
				static std::map<Name,T>& remote_store(){
					static std::map<Name,T> rs;
					return rs;
				}
			};
			
			template<HandleAccess ha, typename T>
			auto newObject(tracker::Tracker &trk, mtl::TransactionContext *tc, Name name, const T& init){
				auto ret = make_handle<l,ha,T,TrackerTestingObject<T> >
					(trk,tc,name,init);
				trk.onCreate(*this,name);
				return ret;
			}

			template<HandleAccess ha, typename T>
			auto existingObject(tracker::Tracker &trk, mtl::TransactionContext *tc, Name name, T* for_inf = nullptr){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				GSQLObject gso(*this,t,name);
				return make_handle
					<l,ha,T,SQLObject<T> >
					(trk,tc,std::move(gso),nullptr);
			}

			template<typename T>
			std::unique_ptr<SQLObject<T> > existingRaw(Name name, T* for_inf = nullptr){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				return std::unique_ptr<SQLObject<T> >
				{new SQLObject<T>{GSQLObject{*this,t,name},nullptr}};
			}


		};
	}}
