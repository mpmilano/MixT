#pragma once

private:

class HandlePrime;
template<typename T>
class HandleImpl;

std::map<uint, std::unique_ptr<HandlePrime> > hndls;
int get_next_id(){
	static int id = 0;
	return ++id;
}

int get_next_rid(){
	static int rinit = 0;
	return ++rinit;
}

class HandlePrime {
private: 
	DataStore& parent;
	virtual bool is_abstract() = 0;
	
	
	HandlePrime(DataStore& newParent, const HandlePrime &old):
		parent(newParent),
		id(old.id),
		rid(old.rid){assert(&old.parent != &parent);}
	
	HandlePrime(DataStore& parent):
		parent(parent),
		id(parent.get_next_id()),
		rid(parent.get_next_rid()){
		//most dangerous game
	}

public:
	const uint id;
	const uint rid;
	
	HandlePrime(const HandlePrime &old) = delete;
	
	virtual ~HandlePrime() {}
	
	virtual void grab_obj(const HandlePrime &) = 0;
	
	template<typename T>
	friend class HandleImpl;
	
};

template<typename T> 
class HandleImpl : public HandlePrime {
	
public:
	std::unique_ptr<T> stored_obj;
private:
	virtual bool is_abstract()  {return false;}
public:
	operator T& (){ return *(this->stored_obj);}
	void operator =(std::unique_ptr<T> n) 
		{this->stored_obj = std::move(n);}
	operator std::unique_ptr<T>() {return std::move(this->stored_obj);}
	
		private:
	HandleImpl(DataStore& parent,std::unique_ptr<T> n):
		HandlePrime(parent),stored_obj(std::move(n)){}
	HandleImpl(DataStore& parent,const HandleImpl& old):
		HandlePrime(parent,old),
		stored_obj(new T(*old.stored_obj)){}
	
	static HandleImpl<T>& place(HandleImpl<T>* hi){
		HandlePrime* h = hi;
		auto old_addr = &(hi->parent.hndls[h->id]);
		hi->parent.hndls[h->id] = std::unique_ptr<HandlePrime>(h);
		assert (old_addr == &(hi->parent.hndls[h->id]));
		return *hi;
	}
public: 
	HandleImpl (const HandleImpl&) = delete;
	
	static HandleImpl<T> & 
	constructAndPlace(DataStore& parent,std::unique_ptr<T> n){
		return place(new HandleImpl<T>(parent, std::move(n)));
	}
	
	static HandleImpl<T> & 
	constructAndPlace(DataStore& parent,const HandleImpl& old) {
		return place(new HandleImpl<T>(parent, old));
	}
	
	auto clone(DataStore& np) const {
		//std::cout << "cloning!" << std::endl;
		auto *h = new HandleImpl<T>(np,*this);
		place(h);
		return std::ref(*h);
	}
	
	virtual void grab_obj(const HandlePrime &hp) {
		//std::cout << "grabbing object!" << std::endl;
		assert(rid == hp.rid); 
		HandleImpl<T> &hi = (HandleImpl<T>&) hp;
		stored_obj.reset(new T(*hi.stored_obj));
	}
	
	
	virtual ~HandleImpl() {}
			
};

public:

class GenericHandle{
private: 
	virtual bool is_virtual() = 0;
};

template <typename T>
class TypedHandle : public GenericHandle{
private: 
	virtual bool is_virtual() = 0;
	HandleImpl<T> &h_i;
	HandleImpl<T> &hi() const {return h_i;}
public:
	TypedHandle(HandleImpl<T> &hi):h_i(hi){}
	friend class DataStore;
	template<Client_Id>
	friend class Client;
};

template<Client_Id id, Level L, HandleAccess HA, typename T>
class Handle : public TypedHandle<T> {
private:
	virtual bool is_virtual() {return false;}
	Handle(HandleImpl<T> &hi):TypedHandle<T>(hi){}
public:
	static constexpr Level level = L;
	static constexpr HandleAccess ha = HA;
	typedef T stored_type;
	friend class DataStore;
	template<Client_Id>
	friend class Client;
};

template<Client_Id id, Level L, HandleAccess HA, typename T>	
auto newhandle_internal(std::unique_ptr<T> r) {
	return Handle<id,L,HA,T>
		(HandleImpl<T>::constructAndPlace(*this,std::move(r)));
}

template<Client_Id id, Level L, HandleAccess HA, typename T>
auto del_internal(Handle<id,L,HA,T> &hndl_i){
	auto &hndl = hndl_i.hi(); 
	std::unique_ptr<T> ret = hndl;
	assert(hndls[hndl.id]->id == hndl.id);
	hndls[hndl.id].first.reset(nullptr);
	return ret;
}
