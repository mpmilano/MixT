#pragma once

#include "Transaction.hpp"
#include "Operation.hpp"
#include "DataStore.hpp"
#include "RemoteObject.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <iostream>
#include <fstream>
#include <set>
#include <cstdlib>
#include <cstring>
#include <boost/filesystem.hpp>
#include <boost/serialization/vector.hpp>
HACKY_PREAMBLE

template<Level l>
struct FileStore : public DataStore<l> {

private:
	FileStore(){
		std::cerr << "YOU REALLY REALLY NEED " <<
			"TO RE-ENABLE THE TRACKER HERE WHEN IT COMES TIME TO TEST THAT CODE"
				  << std::endl;
		/*
		Tracker::global_tracker().registerStore(
			*this,
			[this](const auto &name, const auto &val){
				return newObject<HandleAccess::all>(name,val);
			},
			[this](const auto &name) -> bool{
				return exists(name);
			},
			[this](const auto &name, const auto *inf){
				return existingObject<HandleAccess::all>(name,inf);
			}
			); //*/
	}
	
public:

	FileStore(const FileStore<l>&) = delete;


	static FileStore& filestore_instance() {
		static FileStore fs;
		return fs;
	}

	using id = std::integral_constant<int, (int) l>;
	int ds_id() const { return id::value; }
	
	template<typename T>
	struct FSObject : public RemoteObject<T> {
		std::unique_ptr<T> t;
		const std::string filename;

		typedef FileStore<l> Store;
		Store &s;
		const GDataStore& store() const {return s;}
		GDataStore& store() {return s;}
		
		FSObject(Store &s, const std::string &name, bool exists = false)
			:filename(name),s(s){
			if (!exists){
				std::ofstream ofs(filename);
				boost::archive::text_oarchive oa(ofs);
				FSObject<T> &ths = *this;
				oa << ths;
			}
		}
		
		FSObject(Store &s, const std::string &name, const T &init)
			:t(heap_copy(init)),filename(name),s(s) {
			std::ofstream ofs(filename);
			boost::archive::text_oarchive oa(ofs);
			static_assert(!std::is_const<decltype(this)>::value,"Static assert failed");
			FSObject<T> &ths = *this;
			oa << ths;
		}

		bool ro_isValid() const {return true;}

		TransactionContext* currctx = nullptr;
		
		void setTransactionContext(TransactionContext* a) {
			//TODO: do I want transactions here?
			currctx = a;
		}

		TransactionContext* currentTransactionContext() {
			return currctx;
		}
		
		int to_bytes(char* _v) const {
			((int*)_v)[0] = id::value;
			char* v = _v + sizeof(int);
			if (std::strcpy(v,filename.c_str()))
				return filename.length() + 1 + sizeof(int);
			else assert(false && "error strcpy failed");
		}

		int bytes_size() const {
			return filename.length() + 1 + sizeof(int);
		}

		struct StupidWrapper{
			std::conditional_t<std::is_default_constructible<T>::value, T, T*> val;

			template<typename T2, restrict(std::is_same<decltype(val) CMA T2>::value)>
			StupidWrapper(const T2 &t):val(t) {}

			template<typename T2, restrict2(!std::is_same<decltype(val) CMA T2>::value)>
			StupidWrapper(const T2 &t):val(*t) {}

			StupidWrapper(){}

			template<typename T2>
			static const auto& deref(const T2 *t2) {
				return *t2;
			}

			template<typename T2, restrict(!std::is_pointer<T2>::value)>
			static const auto& deref(const T2 &t2) {
				return t2;
			}

			auto deref() const {
				return deref(val);
			}

		};

		template<class Archive> typename std::enable_if<std::is_pod<T>::value &&
		!std::is_pod<Archive>::value >::type
		save(Archive &ar, const uint) const {
			if (t.get()){
				StupidWrapper stupid(t.get());
				ar << stupid.val;
			}
			else {
				T t;
				ar << t;
			}
		}

		template<class Archive> typename std::enable_if<std::is_pod<T>::value &&
		!std::is_pod<Archive>::value >::type
		load(Archive &ar, const uint){
			StupidWrapper stupid;
			ar >> stupid.val;
			t = heap_copy(stupid.deref());
		}
		
		BOOST_SERIALIZATION_SPLIT_MEMBER()

		template<class Archive> typename std::enable_if<!std::is_pod<T>::value &&
		!std::is_pod<Archive>::value>::type
		save(Archive &ar, const uint) const {
			char *v = (char*) malloc(::bytes_size(*t));
			int size = ::to_bytes(*t,v);
			std::vector<char> v2(v, v + size);
			ar << v2;
			free(v);
		}

		
		template<class Archive> typename std::enable_if<!std::is_pod<T>::value &&
		!std::is_pod<Archive>::value>::type
		load(Archive &ar, const uint) {
			std::vector<char> v;
			ar >> v;
			if (v.size() != 0)
				t.reset(::from_bytes<T>(&v[0]).release());
		}

		virtual const T& get() {
			std::ifstream ifs(filename);
			boost::archive::text_iarchive ia(ifs);
			ia >> *this;
			return *t;
		}

		virtual void put(const T& t) {
			std::ofstream ofs(filename);
			boost::archive::text_oarchive oa(ofs);
			this->t = heap_copy(t);
			oa << *this;
		}

	};
	
	template<typename T>
	struct FSDir : public FSObject<std::set<T> > {
		FSDir(typename FSObject<std::set<T> >::Store &s, const std::string &name)
			:FSObject<std::set<T> >(s,name,true){
			system(("exec mkdir -p " + name).c_str());
		}

		template<class Archive>
		void serialize(Archive &, const uint){
			assert(false && "this should not be serialized");
		}

		const std::set<T>& get() {
			static std::set<T> ret;
			ret.clear();
			for (const auto &str : read_dir(this->filename)){
				FSObject<T> obj(this->s,this->filename + str,true);
				ret.insert(obj.get() );
			}
			return ret;
		}

		virtual void put(const std::set<T> &s) {
			std::system(("exec rm -r " + this->filename + "*").c_str());
			for (const auto &e : s){
				FSObject<T> obj(this->s,this->filename + std::to_string(gensym()) );
				obj.put(e);
			}
		}
	};

	template<typename T>
	static std::unique_ptr<FSObject<T> > from_bytes(char* v) {
		boost::filesystem::path p(v);
		assert(boost::filesystem::exists(p));
		if (boost::filesystem::is_directory(p)){
			assert(false && "can't deserialize dirs yet");
		}
		else {
			return std::make_unique<FSObject<T> >(filestore_instance(),v,true);
		}
	}
	
	template<HandleAccess ha, typename T>
	auto newObject(){
		return make_handle
			<l,ha,T,FSObject<T>>
			(*this,std::string("/tmp/fsstore/") + std::to_string(gensym()));
	}

	template<HandleAccess ha, typename T>
	auto newCollection(){
		return make_handle
			<l,ha,std::set<T>,FSDir<T>>
			(*this,std::string("/tmp/fsstore/") + std::to_string(gensym()) + "/");
	}

	template<HandleAccess ha, typename T>
	auto newObject(const T &t){
		return make_handle
			<l,ha,T,FSObject<T>>
			(*this,std::string("/tmp/fsstore/") + std::to_string(gensym()),t);
	}

	template<HandleAccess ha, typename T>
	auto newObject(const std::string &name, const T &t){
		return make_handle
			<l,ha,T,FSObject<T>>
			(*this,std::string("/tmp/fsstore/") + name,t);
	}

	template<HandleAccess ha, typename T>
	auto existingObject(const std::string &name, const T* inf = nullptr){
		return make_handle
			<l,ha,T,FSObject<T>>
			(*this,std::string("/tmp/fsstore/") + name,true);
	}

	bool exists(const std::string &name) {
		boost::filesystem::path p(name.c_str());
		return boost::filesystem::exists(p);
	}

	template<typename T>
	static FSObject<T>* tryCast(RemoteObject<T>* r){
		if(auto *ret = dynamic_cast<FSObject<T>* >(r))
			return ret;
		else throw Transaction::ClassCastException();
	}

	template<typename T, restrict(!is_RemoteObj_ptr<T>::value)>
	static auto tryCast(T && r){
		return std::forward<T>(r);
	}
	
	std::unique_ptr<TransactionContext> begin_transaction(){
		struct FSContext : public TransactionContext{
			bool commit() { return true; }
			GDataStore& store() {return filestore_instance();}
		};
		//TODO: do I really want to implement transactions over the FS?
		return std::unique_ptr<TransactionContext>(new FSContext());
	}

	template<typename T> 
	OPERATION(Insert, FSObject<std::set<T> >* ro, const T& t){
		if (FSDir<T>* dir = dynamic_cast<FSDir<T>*>(ro)) {
			FSObject<T> obj(ro->s,dir->filename + std::to_string(gensym()),t);
			return true;
		}		
		assert(false && "didn't pass me an FSDIR!");
		return false;
	}
	END_OPERATION

	OPERATION(Increment, FSObject<int> *ro){
		int curr = ro->get();
		ro->put(curr + 1);
		return true;
	}
	END_OPERATION
	
};

typedef FileStore<Level::strong> StrongFileStore;
typedef FileStore<Level::causal> WeakFileStore;

template<Level l, typename T>
using FSObject = typename FileStore<l>::template FSObject<T>;

template<Level l, typename T>
using FSDir = typename FileStore<l>::template FSDir<T>;	


template<typename T>
FINALIZE_OPERATION(Insert, RemoteObject<std::set<T> >*, const T& )
