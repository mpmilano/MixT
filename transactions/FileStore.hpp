#pragma once

#include "Operation.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <iostream>
#include <fstream>

class FileStore {
private:
	template<typename T>
	class FSObject : public RemoteObject<T> {
	private:
		T t;
		const std::string filename;
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
			std::cout << "getting value!" << std::endl;
			//ia >> *this;
			return t;
		}

		void put(const T& t) {
			std::ofstream ofs(filename);
			boost::archive::text_oarchive oa(ofs);
			this->t = t;
			//oa << *this;
			std::cout << "putting value!" << std::endl;
		}
	};
public:

	template<Level l, HandleAccess ha, typename T>
	auto newObject(){
		return make_handle
			<l,ha,T,FSObject>
			(std::string("/tmp/fsstore/") + std::to_string(gensym()));
	}
	
};
