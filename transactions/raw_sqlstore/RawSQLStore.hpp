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

	template<typename Transactor>
	void enter_txn(pqxx::work& w, Transactor& t){
		t(w);
	}

	struct anonymous_transactor : public pqxx::transactor<> {
		virtual void execute(pqxx::work& w) const = 0;
		virtual ~anonymous_transactor(){}
	};
	
	template<typename subclass>
	struct concrete_transactor : public anonymous_transactor{
		static_assert(std::is_base_of<concrete_transactor,subclass>::value );
		void execute(pqxx::work& w) const {
			(*((subclass*)this))(w);
		}
	};
	
	using supports_sql = SupportedOperation<sql_command,SelfType,SelfType,anonymous_transactor>;

	namespace raw_sql_store{
		struct opaque{};
		template<typename label>
		struct RawSQLStore;
		template<typename l>
		struct RawSQLStore<Label<l> > : public DataStore<Label<l> > {
			using label = Label<l>;

			MATCH_CONTEXT(decide_sql_handle){
				MATCHES(opaque) -> RETURN(Handle<label,opaque,supports_sql> );
				template<typename any> MATCHES(any) -> RETURN(Handle<label,any>);
			};
			
			template<typename T> using SQLHandle = MATCH(decide_sql_handle, T);

			template<typename T> using SQLRO = RemoteObject<label,T>;
			template<typename _opaque> struct SQLObject : public SQLRO<opaque>, public std::enable_shared_from_this<SQLObject<_opaque> > {
				static_assert(std::is_same<_opaque,opaque>::value);
				RawSQLStore &parent;

				SQLObject(DECT(parent) &parent):parent(parent){}
				
				bool isValid(mtl::StoreContext<label>*) const { return true;}
				
				std::shared_ptr<const opaque> get(mutils::DeserializationManager<>*, mtl::StoreContext<label>*){
					return std::make_shared<opaque>();
				}
				
				std::shared_ptr<RemoteObject<label,opaque>> create_new(mtl::StoreContext<label>*, const opaque&) const{
					return std::make_shared<SQLObject>(parent);
				}
				
				void put(mtl::StoreContext<label>*,const opaque&){}
				
				std::unique_ptr<LabelFreeHandle<opaque> > wrapInHandle(std::shared_ptr<RemoteObject<label,opaque> > ){
					static_assert(std::is_same<SQLHandle<opaque>, Handle<label,opaque,supports_sql>>::value );
					return std::unique_ptr<LabelFreeHandle<opaque> >{new SQLHandle<opaque>{this->shared_from_this(),parent}};
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

			SQLHandle<opaque> operation(mtl::PhaseContext<label>*, StoreContext& sc, mutils::DeserializationManager<>* ,
										 OperationIdentifier<sql_command>,SQLObject<opaque>& obj, const anonymous_transactor& t){
				t.execute(sc.pq_txn);
				return SQLHandle<opaque>{obj.shared_from_this(), obj.parent};
			}

			SQLHandle<opaque> get_handle(){
				return SQLHandle<opaque>{std::make_shared<SQLObject<opaque>>(*this),*this};
			}
			
		};
	}
}
