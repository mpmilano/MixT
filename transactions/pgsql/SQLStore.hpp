#pragma once
#include <iostream>
#include "pgsql/SQLStore_impl.hpp"
#include "Operations.hpp"
#include "pgsql/SQLTransaction.hpp"

namespace myria { namespace pgsql {

		template<Level l>
		using level_to_label = std::conditional_t<l == Level::causal, Label<causal>, Label<strong> >;
		
		template<Level l>
		using choose_strong = std::integral_constant<bool, l == Level::strong>*;
		template<Level l>
		using choose_causal = std::integral_constant<bool, l == Level::causal>*;

		template<Level l>
		struct SQLContext : public mtl::StoreContext<label> {
				std::unique_ptr<SQLTransaction> i;
				mutils::DeserializationManager & mngr;
				SQLContext(decltype(i) i, mutils::DeserializationManager& mngr):i(std::move(i)),mngr(mngr){}
				DataStore<label>& store() {return dynamic_cast<DataStore<label>&>( i->gstore);}
				bool store_commit() {return i->store_commit();}
			  bool store_abort() {i->store_abort(); return true;}
			};
		
		template<Level l>
		class SQLStore : public SQLStore_impl, public TrackableDataStore<SQLStore<l>,  level_to_label<l> > {
		public:

			using SQLStore_impl::exists;
			//using TrackableDataStore<SQLStore<l>,  level_to_label<l> >::exists;
			
			static constexpr Level level = l;
			using label = level_to_label<l>;
			
			virtual ~SQLStore() {}

			SQLStore(whenpool(GeneralSQLConnectionPool) whennopool(const std::string) &p)
			  :SQLStore_impl(p,*this,l) {
			}

			using deserialization_context = SQLStore;

			using Store = SQLStore;

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

			const std::array<long long, NUM_CAUSAL_GROUPS>& local_time() const {
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

				std::shared_ptr<RemoteObject<label,T> > create_new(mtl::StoreContext<label>* c, const T& t) const {
					return SQLStore::template newObject_static<T> (const_cast<GSQLObject&>(gso).store(),tds,dynamic_cast<SQLContext*>(c),mutils::int_rand(),t);
				}

				const std::array<long long,NUM_CAUSAL_GROUPS>& timestamp() const {
					return gso.timestamp();
				}

				void put(mtl::StoreContext<label>* _tc, const T& t){
					SQLTransaction *tc = (_tc ? ((SQLContext*) _tc)->i.get() : nullptr);
					this->t = std::make_shared<T>(t);
					gso.resize_buffer(mutils::bytes_size(t));
					mutils::to_bytes(t,gso.obj_buffer());
					gso.save(tc);
				}

				bool isValid(mtl::StoreContext<label>* _tc) const{
					SQLTransaction *tc = (_tc ? ((SQLContext*) _tc)->i.get() : nullptr);
					return gso.isValid(tc);
				}
				const SQLStore& store() const{
					return dynamic_cast<SQLStore&>(gso.store());
				}
				SQLStore& store(){
					return dynamic_cast<SQLStore&>(gso.store());
				}
				Name name() const {
					return gso.name();
				}

				INHERIT_SERIALIZATION_SUPPORT(SQLObject, RemoteObject<label,T>, 498645 + l, gso);

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
			static std::shared_ptr<SQLObject<T> > newObject_static(SQLStore_impl& ss, mutils::DeserializationManager& dsm,
																														 SQLContext *ctx, Name name, const T& init){
				constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				std::size_t size = mutils::bytes_size(init);
				std::vector<char> v(size);
				whendebug(std::size_t tb_size) = mutils::to_bytes(init,v.data());
				assert(mutils::bytes_size(init) == size);
				assert(size == tb_size);
				GSQLObject gso(ctx->i.get(),ss,t,name,v);
				return std::make_shared<SQLObject<T> >(std::move(gso),mutils::heap_copy(init),dsm);
			}
			
			template<typename T>
			SQLHandle<T> newObject(SQLContext *ctx, Name name, const T& init){
				return SQLHandle<T>{newObject_static(*this,*this->this_mgr,ctx,name,init),*this};
			}

			template<typename T>
			auto newObject(SQLContext *ctx, const T& init){
				return newObject<T>(ctx,mutils::int_rand(),init);
			}

			template<typename T> SQLHandle<T> nullObject(){
				return SQLHandle<T>{mutils::identity_struct1<SQLObject>{}, *this};
			}

			bool exists(SQLContext* ctx, Name n){
				return exists((ctx ? ctx->i.get() : nullptr),n);
			}

			template<typename T>
			auto existingObject(SQLContext *, Name name){
				static constexpr Table t =
					(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
				GSQLObject gso(*this,t,name);
				return SQLHandle<T>{std::make_shared<SQLObject<T> >(std::move(gso),nullptr,*this->this_mgr),*this};
			}

			using StoreContext = SQLContext<l>;

			std::unique_ptr<mtl::StoreContext<label> > begin_transaction(whendebug(const std::string &why))
				{
					auto ret = SQLStore_impl::begin_transaction(whendebug(why));
					return std::unique_ptr<mtl::StoreContext<label> >(new SQLContext{std::move(ret),*this->this_mgr});
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
#include "tracker/trackable_datastore_impl.hpp"
