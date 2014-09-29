#pragma once
#include "Backend.hpp"
#include "Instance.hpp"
#include "Util.hpp"
#include <memory>

template<location l>
class Instance<l>::StoredBlob{
	typedef int ObjectID;
	virtual ObjectID id() = 0;
};

template<location l>
template<typename T> 
class Instance<l>::StoredObject : public StoredBlob {
private:
	std::unique_ptr<T> t;
	StoredBlob::ObjectID _id;
public:
	StoredObject(std::unique_ptr<T> t):t(std::move(t)),_id(gensym()){}
	
	virtual typename StoredBlob::ObjectID id(){
		return _id;
	}
	
	template<LSWhen>
	friend class LogStore;
};
