#pragma once

#include "Operation.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <iostream>
#include <fstream>
#include <set>
#include <cstdlib>

template<Level l>
struct FileStore {
	template<typename T>
	struct FSObject : public RemoteObject<T> {
		T t;
		const std::string filename;
		
		FSObject(const std::string &name, bool exists = false):filename(name){
			if (!exists){
				std::ofstream ofs(filename);
				boost::archive::text_oarchive oa(ofs);
				oa << *this;
			}
		}

		template<class Archive> typename std::enable_if<std::is_pod<T>::value &&
		!std::is_pod<Archive>::value >::type
		serialize(Archive &ar, const uint){
			ar & t;
		}

		template<class Archive> typename std::enable_if<!std::is_pod<T>::value &&
		!std::is_pod<Archive>::value>::type
		serialize(Archive &ar, const uint){
			assert(false && "unimplemented");
		}

		virtual const T& get() const {
			std::ifstream ifs(filename);
			boost::archive::text_iarchive ia(ifs);
			ia >> *const_cast<FSObject<T>*>(this);
			return t;
		}

		virtual void put(const T& t) {
			std::ofstream ofs(filename);
			boost::archive::text_oarchive oa(ofs);
			this->t = t;
			oa << *this;
		}
	};
	
	template<typename T>
	struct FSDir : public FSObject<std::set<T> > {
		FSDir(const std::string &name):FSObject<std::set<T> >(name,true){
			system(("exec mkdir -p " + name).c_str());
		}

		template<class Archive>
		void serialize(Archive &ar, const uint){
			assert(false && "this should not be serialized");
		}

		const std::set<T>& get() const {
			static std::set<T> ret;
			ret.clear();
			for (const auto &str : read_dir(this->filename)){
				FSObject<T> obj(this->filename + str,true);
				ret.insert(obj.get() );
			}
			return ret;
		}

		virtual void put(const std::set<T> &s) {
			std::system(("exec rm -r " + this->filename + "*").c_str());
			for (const auto &e : s){
				FSObject<T> obj(this->filename + std::to_string(gensym()) );
				obj.put(e);
			}
		}
	};

	template<HandleAccess ha, typename T>
	auto newObject(){
		return make_handle
			<l,ha,T,FSObject<T>>
			(std::string("/tmp/fsstore/") + std::to_string(gensym()));
	}

	template<HandleAccess ha, typename T>
	auto newCollection(){
		return make_handle
			<l,ha,std::set<T>,FSDir<T>>
			(std::string("/tmp/fsstore/") + std::to_string(gensym()) + "/");
	}
	
};

template<typename T, typename E>
OPERATION(Insert, RemoteObject<std::set<T> >* ro, E t){

	if (FileStore<Level::causal>::FSDir<T>* dir = dynamic_cast<FileStore<Level::causal>::FSDir<T>*>(ro)) {
		FileStore<Level::causal>::FSObject<T> obj(dir->filename + std::to_string(gensym()));
		obj.put(t);
		return true;
	}
	else if (FileStore<Level::strong>::FSDir<T>* dir = dynamic_cast<FileStore<Level::strong>::FSDir<T>*>(ro)) {
		FileStore<Level::strong>::FSObject<T> obj(dir->filename + std::to_string(gensym()));
		obj.put(t);
		return true;		
	}

	assert(false && "didn't pass me an FSDIR!");
	return false;

}
END_OPERATION
