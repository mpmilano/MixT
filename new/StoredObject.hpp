#pragma once
#include "Backend.hpp"
#include "Instance.hpp"

template<location l>
class Instance<l>::StoredBlob{
	private:
		virtual bool is_abstract() = 0;
		
		static int get_next_id(){
			static int id = 0;
			return ++id;
		}
		StoredBlob(DataStore& newParent, const HandlePrime &old):
			parent(newParent),
			id(old.id){assert(&old.parent != &parent);}
		
		HandlePrime(DataStore& parent):
			parent(parent),
			id(get_next_id()),
			rid(get_next_rid()){
		}
		
	public:
		
		DataStore& parent;
		const uint id;
		
		HandlePrime(const HandlePrime &old) = delete;
	
		virtual ~HandlePrime() {}
		
		virtual void grab_obj(const HandlePrime &) = 0;
	
		template<typename T>
		friend class DataStore::HandleImpl;
	
};

template<location l>
template<typename T> 
class Instance<l>::StoredObject {
	std::unique_ptr<T> t;
	
};
