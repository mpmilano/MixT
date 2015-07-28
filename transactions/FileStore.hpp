#pragma once

#include "Operation.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <iostream>
#include <fstream>

class FileStore {
private:
	template<typename T>
	class FSObject : RemoteObject<T> {
	private:
		T t;
		std::string filename;
	public:

		FSObject(const std::string &name):filename(name){}

		template<class Archive>
		void serialize(Archive &ar, const uint){
			ar & t;
		}

		const T& get() const {
			std::ifstream ifs(filename);
			boost::archive::text_iarchive ia(ifs);
			// read class state from archive
			ia >> *this;
			return t;
		}

		void put(const T& t) {
			std::ofstream ofs(filename);
			boost::archive::text_oarchive oa(ofs);
			this->t = t;
			oa << *this;
		}
	};
public:

	template<Level l, HandleAccess ha, typename T>
	auto newObject(const T &t){
		return make_handle
			<l,ha,T,FSObject>
			(std::string("/tmp/fsstore/") + std::to_string(gensym()));
	}
	
};
