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

template<Level l>
struct FileStore : public DataStore<l> {

private:
	FileStore(){}
	
public:

	FileStore(const FileStore<l>&) = delete;


	static FileStore& filestore_instance() {
		static FileStore fs;
		return fs;
	}

	using id = std::integral_constant<int, (int) l>;
	
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

		bool isValid() const {return true;}
		
		int to_bytes(char* _v) const {
			((int*)_v)[0] = id::value;
			char* v = _v + sizeof(int);
			std::cout << "Serializing this name: " << filename.c_str() << std::endl;
			if (std::strcpy(v,filename.c_str()))
				return filename.length() + 1;
			else assert(false && "error strcpy failed");
		}

		int bytes_size() const {
			return filename.length() + 1;
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
			t.reset(heap_copy(stupid.deref()));
		}
		
		BOOST_SERIALIZATION_SPLIT_MEMBER()

		template<class Archive> typename std::enable_if<!std::is_pod<T>::value &&
		!std::is_pod<Archive>::value>::type
		save(Archive &ar, const uint) const {
			char *v = (char*) malloc(::bytes_size(*t));
			std::cout << "serializing member!" << std::endl;
			::to_bytes(*t,v);
			std::cout << "copying to vector!" << std::endl;
			std::vector<char> v2(v, v + ::bytes_size(*t));
			ar << v2;
			free(v);
		}

		
		template<class Archive> typename std::enable_if<!std::is_pod<T>::value &&
		!std::is_pod<Archive>::value>::type
		load(Archive &ar, const uint) {
			std::vector<char> v;
			ar >> v;
			if (v.size() != 0)
				t.reset(::from_bytes<T>(&v[0]));
		}

		virtual const T& get() const {
			std::ifstream ifs(filename);
			boost::archive::text_iarchive ia(ifs);
			ia >> *const_cast<FSObject<T>*>(this);
			return *t;
		}

		virtual void put(const T& t) {
			std::ofstream ofs(filename);
			boost::archive::text_oarchive oa(ofs);
			this->t.reset(heap_copy(t));
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

		const std::set<T>& get() const {
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
	static FSObject<T>* from_bytes(char* v) {
		boost::filesystem::path p(v);
		assert(boost::filesystem::exists(v));
		std::cout << "Attempting to deserialize " << v << std::endl;
		if (boost::filesystem::is_directory(p)){
			assert(false && "can't deserialize dirs yet");
		}
		else {
			return new FSObject<T>(filestore_instance(),v,true);
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

	bool in_trans = false;
	
	bool in_transaction() const {
		return in_trans;
	}

	void begin_transaction(){
		in_trans = true;
		//TODO: do I really want to implement transactions over the FS?
	}

	void end_transaction(){
		//TODO: do I really want to implement transactions over the FS?
		in_trans = false;
	}

	template<typename T> 
	OPERATION(Insert, FSObject<std::set<T> >* ro, const T& t){
		std::cout << "DOING INSERT!" << std::endl;
		if (FSDir<T>* dir = dynamic_cast<FSDir<T>*>(ro)) {
			FSObject<T> obj(ro->s,dir->filename + std::to_string(gensym()),t);
			std::cout << "Done insert!" << std::endl;
			return true;
		}
		
		assert(false && "didn't pass me an FSDIR!");
		return false;
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
