#pragma once
#include "mtl/top.hpp"
#include "mtl/CTString_macro.hpp"
#include "mtl/CTString.hpp"
#include "mtl/TransactionContext.hpp"
#include "mutils-containers/HeteroMap.hpp"
#include "DataStore.hpp"
#include "Handle.hpp"
#include "Operations.hpp"
#include "tracker/trackable_datastore_impl.hpp"
#include <map>

//I'm sure I've written this a dozen times, but here's another one.  This is supposed to be a local-only store used for testing
//MTL, and anything else that doesn't actually need a backend to correct against.

namespace myria {

	using noop = MUTILS_STRING(noop);
	
	template<>
	struct OperationIdentifier<noop>{
		using name = noop;
	};
	
	namespace testing_store{
		
		template<typename label>
		struct TestingStore;
		template<typename l>
		struct TestingStore<Label<l> > : public TrackableDataStore<TestingStore<Label<l> >, Label<l> >{
			using label = Label<l>;
			template<typename T>
				using TestingHandle =
				Handle<label,T,SupportedOperation<noop,void,SelfType,int,int,int,int> >;

			struct TestingContext;

			mutils::HeteroMap<Name> object_store;
			template<typename T> using stored = std::shared_ptr<T>;
			
			template<typename T>
			struct TestingObject : public RemoteObject<label,T>{
				using Store = TestingStore;
				using stored = Store::template stored<T>;
				Name _name;
				TestingStore &parent;
				TestingObject(Name name, TestingStore &parent)
					:_name(name),parent(parent){}
				
				std::shared_ptr<const T> get(mtl::StoreContext<label>*){
					return *parent.object_store. template at<stored >(_name);
				}

				const std::array<long long, NUM_CAUSAL_GROUPS>& timestamp() const {
					static const std::array<long long, NUM_CAUSAL_GROUPS> ret{{0,0,0,0}};
					return ret;
				}

				void put(mtl::StoreContext<label>*, const T& t){
					auto &ptr = parent.object_store.template mut<stored >(_name);
					if (!ptr) ptr.reset(new stored{new T{t}});
					else if (!*ptr) *ptr = std::make_shared<T>(t);
					else **ptr = t;
					assert(parent.object_store.template mut<stored >(_name));
					assert(*parent.object_store.template mut<stored >(_name));
				}

				bool isValid(mtl::StoreContext<label>*) const{
					return parent.object_store.contains(_name);
				}
				const Store& store() const{
					return parent;
				}
				Store& store(){
					return parent;
				}
				Name name() const {
					return _name;
				}
				std::size_t bytes_size() const {
					assert(false && "serialize this later");
					return 0;
				}
				std::size_t to_bytes(char*) const {
					assert(false && "serialize this later");
					return 0;
				}
				void post_object(const std::function<void (char const * const,std::size_t)>&) const {
					assert(false && "serialize this later");
				}
#ifndef NDEBUG
				void ensure_registered(mutils::DeserializationManager &){}
#endif
				
			};

			struct TestingContext : public mtl::StoreContext<label> {
				TestingStore& parent;
				TestingContext(TestingStore& parent):parent(parent){}
				DataStore<label>& store(){
					return parent;
				}

				bool store_commit() {
					//we always commit
					return true;
				}

				bool store_abort() {
					//we never abort
					assert(false && "we never abort");
					return false;
				}
				
			};

			using StoreContext = TestingContext;
			
			template<typename T>
			TestingHandle<T> newObject(TestingContext *ctx, Name name, const T& init){
				auto sp = std::make_shared<TestingObject<T> >(name,*this);
				sp->put(ctx,init);
				return TestingHandle<T>{sp,*this};
			}

			template<typename T>
			TestingHandle<T> existingObject(TestingContext *, Name name){
				auto sp = std::make_shared<TestingObject<T> >(name,*this);
				return TestingHandle<T>{sp,*this};
			}

			bool exists(TestingContext*, Name n) const {
				return object_store.contains(n);
			}
			
			std::unique_ptr<mtl::StoreContext<label> > begin_transaction(whendebug(const std::string&)){
				return std::make_unique<TestingContext>(*this);
			}

			template<typename T>
			void operation(mtl::PhaseContext<label>*, StoreContext&, OperationIdentifier<noop>,TestingObject<T>&,int,int,int,int){
			}

#ifndef NDEBUG
			std::string why_in_transaction() const {
				return "not in testing store";
			}
#endif
		};
	}}
