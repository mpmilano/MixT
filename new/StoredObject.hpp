#pragma once
#include "Backend.hpp"
#include "Instance.hpp"
#include "Util.hpp"
#include <memory>
#include <cassert>

template<location l>
class Instance<l>::StoredBlob{
public:
	typedef int ObjectID;
	virtual ObjectID id() = 0;
	virtual void reset() = 0;
	virtual void checkpoint() = 0;
	virtual void overwrite(StoredBlob &sb) = 0;
};

template<location l>
template<typename T> 
class Instance<l>::StoredObject : public StoredBlob {
private:
	T orig;
	T curr;
	typename StoredBlob::ObjectID _id;
public:
	StoredObject(T &t):orig(t),curr(t),_id(gensym()){}
	StoredObject(T &&t):orig(t),curr(orig),_id(gensym()){}

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

	virtual void overwrite(StoredBlob &sb) {
		auto so = dynamic_cast<StoredObject<T>*>(&sb);
		assert(so);
		orig = so->orig;
		curr = so->curr;
	}

	friend class LogStore;
};
