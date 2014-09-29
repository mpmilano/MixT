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
		return *(h.obj.curr);
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

	template<location l2>
	Instance<l2>::LogStore&& sendTo() const{
		assert(false && "todo");
		return *((Instance<l2>::LogStore*)nullptr)
	}

	void take_objs(std::list<std::unique_ptr<StoredBlob b> > &l){
		assert(false && "todo");
		//needs to overwrite all objects that are applicable, keep
		//the objects which aren't. Assuming we're overwriting from
		//the original list anyway.
	}

	template<location l2>
	fill_in(const Instance<l2>::LogStore &from){
		static_assert(l2 != l, "This was intended for replacing local state with remote state.");
		assert(from.deltas.empty());
		LogStore ls = from.sendTo<l>();
		for (auto &o : objs){
			o.reset();
		}
		take_objs(ls.objs);
		auto lsid = ls.idmap;
		auto oldid = this->idmap;
		auto idmap = [oldid, lsid](typename GenericHandle::HandleID id){
			auto tmp = lsid(id);
			if (tmp) return tmp;
			else return oldid(id);
		};
		this->idmap = idmap;

		for (auto& f : deltas) f();
		deltas.clear();
		for (auto &o : objs){
			o.checkpoint();
		}
		
	}

	//THIS DOES NOT CONSTITUTE AN 
	//OWNERSHIP CHANGE.  IT'S JUST
//TO CONVERT TO A MONOID.
	template<typename T>
	T* get (TypedHandle<T> h, T* init){
		if (contains_obj(h.id()) ){
			assert(obj_matches(h));
			return h.obj.curr.get();
		}
		else return init;
	}
};
