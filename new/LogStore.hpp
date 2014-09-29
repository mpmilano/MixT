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

	std::list<std::function<void ()> > deltas;

	std::list<std::unique_ptr<StoredBlob> > objs;

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
		return h.obj.curr;
	}

	static bool containsHandle(const typename StoredBlob::ObjectID){
		return false;
	}

	template<typename... Args>
	static bool containsHandle(const typename StoredBlob::ObjectID hid, const GenericHandle& h, Args... a){
		return h.id() == hid || containsHandle(hid,a...);
	}

	static std::unique_ptr<GenericHandle> findHandle(const typename StoredBlob::ObjectID){
		assert(false && "Recursion bottomed out!");
		return std::unique_ptr<GenericHandle>(nullptr);
	}


	template<typename T_, typename... Args>
	static std::unique_ptr<GenericHandle> findHandle(const typename StoredBlob::ObjectID hid, const TypedHandle<T_> h, Args... hrest){
		return 
			hid == h.id() ? std::unique_ptr<GenericHandle>(new TypedHandle<T_>(h))
			: findHandle(hid,hrest...);
	}

	
public:

	template<typename T>
	TypedHandle<T> takeObj(T b){
		auto tmp = new StoredObject<T>(std::move(b));
		objs.push_back(std::unique_ptr<StoredBlob>(tmp));
		return TypedHandle<T>(*tmp);
	}
	
	template<typename F, typename... Args>
	void add(F f, Args... params){
		//Params must be handles, but this is enforced via call to freshen anyway
		std::function<void ()> update = [this,f,params...]() {
			f(get_obj(params)...);
		};
		update();
		deltas.push_back(update);
	}

	template<location l2>
	typename Instance<l2>::LogStore&& sendTo() const{
		assert(false && "todo");
		return *( (typename Instance<l2>::LogStore*) nullptr);
	}

	void take_objs(std::list<std::unique_ptr<StoredBlob > > &){
		assert(false && "todo");
		//needs to overwrite all objects that are applicable, keep
		//the objects which aren't. Assuming we're overwriting from
		//the original list anyway.
		//make sure you don't do this in a way that invalidates the 
		//existing handles!
	}

	template<location l2>
	void fill_in(const typename Instance<l2>::LogStore &from){
		static_assert(l2 != l, "This was intended for replacing local state with remote state.");
		assert(from.deltas.empty());
		LogStore ls = from.sendTo();
		for (auto &o : objs){
			o.reset();
		}
		take_objs(ls.objs);

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
			return &(h.obj.curr);
		}
		else return init;
	}
};
