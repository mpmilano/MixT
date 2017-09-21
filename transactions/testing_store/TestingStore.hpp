#pragma once
#include "mtl/top.hpp"
#include "mutils/CTString_macro.hpp"
#include "mutils/CTString.hpp"
#include "mtl/TransactionContext.hpp"
#include "mutils-containers/HeteroMap.hpp"
#include "DataStore.hpp"
#include "Handle.hpp"
#include "Operations.hpp"
#include "tracker/trackable_datastore_impl.hpp"
#include <map>
#include "mutils-serialization/SerializationSupport.hpp"

//I'm sure I've written this a dozen times, but here's another one.  This is supposed to be a local-only store used for testing
//MTL, and anything else that doesn't actually need a backend to correct against.

namespace myria {

	using noop = MUTILS_STRING(noop);
	using append = MUTILS_STRING(append);
	using consistent_read = MUTILS_STRING(consistent_read);
	using increment = MUTILS_STRING(increment);
	
	template<>
	struct OperationIdentifier<noop>{
		using name = noop;
	};

	template<>
	struct OperationIdentifier<append>{
		using name = append;
	};

	template<>
	struct OperationIdentifier<consistent_read>{
		using name = consistent_read;
	};

	using supports_increment = SupportedOperation<increment,void,SelfType>;
	using supports_noop = SupportedOperation<noop,void,SelfType,int,int,int,int>;
	template<typename T> using supports_append = SupportedOperation<append,void,SelfType,T>;
	template<typename T> using supports_consistent_read = SupportedOperation<consistent_read,T,SelfType>;
	
	namespace testing_store{
		
		template<typename label>
		struct TestingStore;
		template<typename l>
		struct TestingStore<Label<l> > : public TrackableDataStore<TestingStore<Label<l> >, Label<l> >{
			using label = Label<l>;

			MATCH_CONTEXT(decide_testing_handle){
				template<typename U> MATCHES(std::list<U>) -> RETURN(Handle<label,std::list<U>,supports_noop, supports_consistent_read<std::list<U> >, supports_append<U> >);
				template<typename T> MATCHES(T) -> RETURN(Handle<label,T,supports_noop, supports_consistent_read<T> >);
				MATCHES(int) -> RETURN(Handle<label,int,supports_noop, supports_consistent_read<int>, supports_increment>);
			};

			template<typename T> using TestingHandle = MATCH(decide_testing_handle, T);
			
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

				std::shared_ptr<RemoteObject<label,T> > create_new(mtl::StoreContext<label>* ctx, const T& t) const {
					return parent.newObject<T>(dynamic_cast<TestingContext*>(ctx),124,t)._ro;
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
				template<typename... ctxs>
				void ensure_registered(mutils::DeserializationManager<ctxs...> &){}
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
			TestingHandle<T> newObject(const T& init){
				return newObject(nullptr, mutils::int_rand(), init);
			}

			template<typename T> TestingHandle<T> nullObject(){
				return TestingHandle<T>{mutils::identity_struct1<TestingObject>{}, *this};
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
				std::cout << "operation executed" << std::endl;
			}
			
			template<typename T>
			void operation(mtl::PhaseContext<label>*, StoreContext&, OperationIdentifier<append>,TestingObject<std::list<T> >& hndl,const T& t){
				std::cout << "append executed" << std::endl;
				std::list<T> &lst = **object_store. template at<stored<std::list<T>>>(hndl._name);
				lst.push_back(t);
			}

			template<typename T>
			void operation(mtl::PhaseContext<label>* , StoreContext& ctx, OperationIdentifier<consistent_read>,TestingObject<T>& hndl){
				std::cout << "consistent read" << std::endl;
				return hndl.get(&ctx);
			}

			void operation(mtl::PhaseContext<label>* , StoreContext& ctx, OperationIdentifier<consistent_read>,TestingObject<int>& hndl){
				return hndl.put(&ctx,hndl.get(&ctx)+1);
			}


#ifndef NDEBUG
			std::string why_in_transaction() const {
				return "not in testing store";
			}
#endif
		};
	}}
