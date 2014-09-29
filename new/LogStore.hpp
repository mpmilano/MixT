#pragma once
#include "Backend.hpp"
#include "Instance.hpp"
#include "StoredObject.hpp"
#include "Handle.hpp"
#include <memory>
#include <functional>
#include <list>
#include <cassert>

template<location l>
class Instance<l>::LogStore{
private:

	static std::unique_ptr<GenericHandle> init_idmap(typename GenericHandle::HandleID) { 
		return std::unique_ptr<GenericHandle>(nullptr); 
	}

	std::function< std::unique_ptr<GenericHandle> 
				   (typename GenericHandle::HandleID)> idmap;
		

	std::list<std::function<void ()> > deltas;

	std::list<std::unique_ptr<StoredBlob> > objs;
	
	template<typename T>
	TypedHandle<T> freshen_handles(TypedHandle<T> t){
		auto uptr = idmap(t.id());
		auto ptr = uptr.get();
		auto ret = dynamic_cast<TypedHandle<T>*>(ptr);
		assert(ret);
		return *ret;
	}

	bool contains_obj (typename StoredBlob::ObjectID id) const {
		for (auto &o : objs){
			if (id == o->id()) return true;
		}
		return false;
	}

	//for sanity checking.
	template<typename T>
	bool obj_matches(const TypedHandle<T> &h) const{
		for (auto &o : objs){
			if (o->id() == h.id())
				assert(o.get() == &(h.obj));
		}
		return true;
	}


	template<typename T>
	T& get_obj(TypedHandle<T> h){
		assert(contains_obj(h.id()));
		assert(obj_matches(h));
		return *(h.obj.t);
	}

	static bool containsHandle(const typename GenericHandle::HandleID){
		return false;
	}

	template<typename... Args>
	static bool containsHandle(const typename GenericHandle::HandleID hid, const GenericHandle& h, Args... a){
		return h.id() == hid || containsHandle(hid,a...);
	}

	static std::unique_ptr<GenericHandle> findHandle(const typename GenericHandle::HandleID){
		assert(false && "Recursion bottomed out!");
		return std::unique_ptr<GenericHandle>(nullptr);
	}


	template<typename T_, typename... Args>
	static std::unique_ptr<GenericHandle> findHandle(const typename GenericHandle::HandleID hid, const TypedHandle<T_> h, Args... hrest){
		return 
			hid == h.id() ? std::unique_ptr<GenericHandle>(new TypedHandle<T_>(h))
			: findHandle(hid,hrest...);
	}

	
public:

	LogStore():idmap(init_idmap){}

	template<typename T>
	TypedHandle<T> takeObj(std::unique_ptr<T> b){
		auto tmp = new StoredObject<T>(std::move(b));
		objs.push_back(std::unique_ptr<StoredBlob>(tmp));
		return TypedHandle<T>(*tmp);
	}

	template<typename T>
	TypedHandle<T> takeObj(StoredObject<T> b) {
		auto tmp = new StoredObject<T>(std::move(b));
		objs.push_back(std::unique_ptr<StoredBlob>(tmp));
		return TypedHandle<T>(*tmp);
	}
	
	template<typename F, typename... Args>
	void add(F f, Args... params){
		//Params must be handles, but this is enforced via call to freshen anyway
		auto idmap = this->idmap;
		auto newmap = [idmap, params...](typename GenericHandle::HandleID id){
			if (containsHandle(id, params...)){
				return findHandle(id, params...);
			}
			else return idmap(id);
		};
		this->idmap = newmap;
		std::function<void (initial_value)> update = [this,f,params...]() {
			f(get_obj(freshen_handles(params)...));
		};
		if (when == LSWhen::immediate) update();
		else deltas.push_back(update);
	}

	template<location l>
	flush(Instance<l>::LogStore &to){
		auto idmap = [&](typename GenericHandle::HandleID id){
			auto tmp = to.idmap(id);
			if (tmp) return tmp;
			else assert(false && "TODO");
		};
		this->idmap = idmap;

		for (auto& f : deltas) f();
		deltas.clear();
		idmap = init_idmap;
	}

	template<location l2>
	typename std::enable_if<when == LSWhen::immediate>::type
	replaceWith(typename Instance<l2>::template LogStore<LSWhen::immediate> &){
		static_assert(l2 != l, "This was intended for replacing local state with remote state.");
		//TODO body here
	}

	//THIS DOES NOT CONSTITUTE AN 
	//OWNERSHIP CHANGE.  IT'S JUST
//TO CONVERT TO A MONOID.
	template<typename T>
	T* get (TypedHandle<T> h, T* init){
		if (contains_obj(h.id()) ){
			assert(obj_matches(h));
			return h.obj.t.get();
		}
	}
};
