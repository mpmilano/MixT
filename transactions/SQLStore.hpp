#pragma once
#include "SQLStore_impl.hpp"
#include "Tracker_common.hpp"
#include "Tracker_support_structs.hpp"

template<Level l>
class SQLStore : public SQLStore_impl, public DataStore<l> {

    SQLStore(int inst_id):SQLStore_impl(*this,inst_id,l) {
		std::unique_ptr<Tracker::TrackerDS<l> > (*f) (Tracker::replicaID) =
			[](Tracker::replicaID i) -> std::unique_ptr<Tracker::TrackerDS<l> >
			{return wrapStore(inst(i));};
		Tracker::global_tracker().registerStore(
			*this,f);
	}
public:

	using Store = SQLStore;
	
	static SQLStore& inst(int instance_id){
		static std::map<int,SQLStore* > ss;
		if (ss.count(instance_id) == 0){
            ss[instance_id] = new SQLStore(instance_id);
		}
		return *ss.at(instance_id);
	}

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

	template<typename T>
	struct SQLObject : public RemoteObject<T> {
		using Store = SQLStore;
		GSQLObject gso;
		std::unique_ptr<T> t;

		SQLObject(GSQLObject gs, std::unique_ptr<T> t):
			gso(std::move(gs)),t(std::move(t)){}

		const T& get(Tracker *trk){
			choose_strong<l> choice {nullptr};
			return get(trk,choice);
		}
		
		const T& get(Tracker *, std::true_type*) {
			assert(l == Level::strong);
			char * res = nullptr;
			res = gso.load();
			assert(res);
			if (res != nullptr){
				t = ::from_bytes<T>(res);
			}
			return *t;
		}

		const T& get(Tracker* trk, std::false_type*){
			//if no tracking instance given,
			//then just assume we're in some
			//metadata case and don't try and track.
			assert(l == Level::causal);
			if (trk){
				t = trk->template onRead<T,SQLStore<Level::causal>::SQLObject>
					(store(),name());
				return *t;
			}
			else {
				std::true_type* choice{nullptr};
				return get(trk,choice);
			}
		}

		void put(const T& t){
			this->t = std::make_unique<T>(t);
			::to_bytes(t,gso.obj_buffer());
			gso.save();
		}

		//these just forward
		void setTransactionContext(TransactionContext* t){
			gso.setTransactionContext(t);
		}
		TransactionContext* currentTransactionContext(){
			return gso.currentTransactionContext();
		}
		bool ro_isValid() const{
			return gso.ro_isValid();
		}
		const SQLStore& store() const{
			return SQLStore::inst(gso.store_instance_id());
		}
		SQLStore& store(){
			return SQLStore::inst(gso.store_instance_id());
		}
		int name() const {
			return gso.name();
		}
		int bytes_size() const {
			return gso.bytes_size();
		}
		int to_bytes(char* c) const {
			return gso.to_bytes(c);
		}
	};

	template<typename T>
	static SQLObject<T>* tryCast(RemoteObject<T>* r) {
		if(auto *ret = dynamic_cast<SQLObject<T>* >(r))
			return ret;
		else throw Transaction::ClassCastException();
	}
	
	template<typename T, restrict(!is_RemoteObj_ptr<T>::value)>
	static auto tryCast(T && r){
		return std::forward<T>(r);
	}

	template<HandleAccess ha, typename T>
	auto newObject(const T& init){
		static constexpr Table t =
			(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
		int size = ::bytes_size(init);
		std::vector<char> v(size);
		assert(size == ::to_bytes(init,&v[0]));
		GSQLObject gso(*this,t,rand(),v);
		return make_handle
			<l,ha,T,SQLObject<T> >
			(std::move(gso),heap_copy(init) );
	}
	
	template<HandleAccess ha, typename T>
	auto newObject(int name, const T& init){
		static constexpr Table t =
			(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
		int size = ::bytes_size(init);
		std::vector<char> v(size);
		assert(size == ::to_bytes(init,&v[0]));
		GSQLObject gso(*this,t,name,v);
		return make_handle
			<l,ha,T,SQLObject<T> >
			(std::move(gso),heap_copy(init) );
	}

	template<HandleAccess ha, typename T>
	auto existingObject(int name, T* for_inf = nullptr){
		static constexpr Table t =
			(std::is_same<T,int>::value ? Table::IntStore : Table::BlobStore);
		GSQLObject gso(*this,t,name);
		return make_handle
			<l,ha,T,SQLObject<T> >
			(std::move(gso),nullptr);
	}

	template<typename T>
	static std::unique_ptr<SQLObject<T> > from_bytes(char* v){
		return std::make_unique<SQLObject<T> >(GSQLObject::from_bytes(v),
											   std::unique_ptr<T>());
	}

	std::unique_ptr<TransactionContext> begin_transaction(){
        auto ret = SQLStore_impl::begin_transaction();
        return ret;
	}

	int instance_id() const {
		return SQLStore_impl::instance_id();
	}

	OPERATION(Increment, SQLObject<int>* o) {
		o->gso.increment();
		return true;
	}
	END_OPERATION
};
