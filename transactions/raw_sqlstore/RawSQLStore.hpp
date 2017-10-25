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
#include <pqxx/pqxx>

/*raw SQL store: for just directly using the PQXX stuff with minimal interruption from our library*/

namespace myria{
	using sql_command = MUTILS_STRING(sql_command);
	template<>
	struct OperationIdentifier<sql_command>{
		using name = sql_command;
	};
	
	struct Unsupported{};

	template<typename transactor>
	using supports_sql = SupportedOperation<sql_command,SelfType,SelfType,transactor>;

	namespace raw_sql_store{
		template<typename label, typename... Transactors>
		struct RawSQLStore;
		template<typename l, typename... Transactors>
		struct RawSQLStore<Label<l>, Transactors... > : public DataStore<Label<l> > {
			using label = Label<l>;
			
			template<typename T> using SQLHandle = Handle<label,T,supports_sql<Transactors>...>;

			template<typename T> using SQLRO = RemoteObject<label,T>;
			template<typename T> struct SQLObject : public SQLRO<T>, public std::enable_shared_from_this<SQLObject<T> > {
				RawSQLStore &parent;
				std::unique_ptr<T> data;

				SQLObject(DECT(parent) &parent, std::unique_ptr<T> data):parent(parent),data(std::move(data)){}
				
				bool isValid(mtl::StoreContext<label>*) const { return true;}
				
				std::shared_ptr<const T> get(mutils::DeserializationManager<>*, mtl::StoreContext<label>*){
					throw Unsupported{};
				}
				
				std::shared_ptr<RemoteObject<label,T>> create_new(mtl::StoreContext<label>*, const T& t) const{
					return std::make_shared<SQLObject>(parent, std::make_unique<T>(t));
				}
				
				void put(mtl::StoreContext<label>*,const T&){
					throw Unsupported{};
				}
				
				std::unique_ptr<LabelFreeHandle<T> > wrapInHandle(std::shared_ptr<RemoteObject<label,T> > ){
					return std::unique_ptr<LabelFreeHandle<T> >{new SQLHandle<T>{this->shared_from_this(),parent}};
				}
				
				const RawSQLStore& store() const {
					return parent;
				}
				RawSQLStore& store(){
					return parent;
				}
				Name name() const{
					return 0;
				}
				const std::array<long long,NUM_CAUSAL_GROUPS>& timestamp() const {
					static std::array<long long,NUM_CAUSAL_GROUPS> ret{{0,0,0,0}};
					return ret;
				}
				std::size_t serial_uuid() const {
					return 14312;
				}
				std::size_t bytes_size() const {return 1;}
				std::size_t to_bytes(char* c) const {
					c[0] = true;
					return 1;
				}
				template<typename... ctxs>
				static std::unique_ptr<SQLObject> from_bytes(mutils::DeserializationManager<ctxs...>* dsm, char const * const ){
					return std::unique_ptr<SQLObject>{new SQLObject{dsm->template mgr<RawSQLStore>()}};	
				}
				
			};
			using InheritPair = mutils::InheritPairAbs1<SQLRO, SQLObject, 14312>;

			struct SQLContext : public mtl::StoreContext<label>{
				RawSQLStore &parent;
				pqxx::work pq_txn;
				template<typename... Args> SQLContext(DECT(parent) &p, Args && ... args)
					:parent(p),pq_txn(std::forward<Args>(args)...){}
				RawSQLStore& store() {return parent;}
				bool store_commit(){
					pq_txn.commit();
					return true;
				}
				bool store_abort(){
					pq_txn.abort();
					return true;
				}
			};
			using StoreContext = SQLContext;

			pqxx::connection conn;
			//arguments to constructor are passed directly to conn.
			template<typename... Args>
			RawSQLStore(Args&&... args)
				:conn(std::forward<Args>(args)...){}
			RawSQLStore(const RawSQLStore&) = delete;
			
			whendebug(std::string why_current_transaction);

			std::unique_ptr<mtl::StoreContext<label>> begin_transaction(whendebug(const std::string &why)){
				whendebug(why_current_transaction = why);
				auto ret = new SQLContext{*this,conn};
				return std::unique_ptr<mtl::StoreContext<label>>{ret};
			}
#ifndef NDEBUG
			std::string why_in_transaction() const {
				return why_current_transaction;
			}
#endif

			template<typename T, typename transactor>
			SQLHandle<T> operation(mtl::PhaseContext<label>*, StoreContext& sc, mutils::DeserializationManager<>* ,
										 OperationIdentifier<sql_command>,SQLObject<T>& obj, const transactor& t){
				t(sc.pq_txn, obj.data);
				return SQLHandle<T>{obj.shared_from_this(), obj.parent};
			}

			template<typename T>
			SQLHandle<T> get_handle(std::unique_ptr<T> o){
				return SQLHandle<T>{std::make_shared<SQLObject<T>>(*this,std::move(o)),*this};
			}
			
		};
	}
}
