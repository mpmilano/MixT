#pragma once
#include "Backend.hpp"
#include "Instance.hpp"
#include "Util.hpp"
#include <memory>

template<location l>
class Instance<l>::StoredBlob{
	typedef int ObjectID;
	virtual ObjectID id() = 0;
	virtual void reset() = 0;
	virtual void checkpoint() = 0;
};

template<location l>
template<typename T> 
class Instance<l>::StoredObject : public StoredBlob {
private:
	T orig;
	T curr;
	StoredBlob::ObjectID _id;
public:
	StoredObject(T &t):orig(t),curr(t),_id(gensym()){}

	StoredObject(const StoredObject&) = delete;
	
	virtual typename StoredBlob::ObjectID id(){
		return _id;
	}

	virtual void reset(){
		curr = orig;
	}

	virtual void checkpoint(){
		orig = curr;
	}
	
	template<LSWhen>
	friend class LogStore;
};
