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

	StoredBlob& get_obj(typename StoredBlob::ObjectID id){
		for (auto &o : objs){
			if (o->id() == id)
				return *o;
		}
		assert(false && "get_obj called on absent object!");
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

	void take_objs(const LogStore &from){
		for (auto &o : objs){
			if (from.contains_obj(o->id()))
				o->overwrite(from.get_obj(o->id()));
		}
	}

	template<location l2>
	void fill_in(const typename Instance<l2>::LogStore &from){
		static_assert(l2 != l, "This was intended for replacing local state with remote state.");
		assert(from.deltas.empty());
		LogStore ls = from.sendTo();
		for (auto &o : objs){
			o.reset();
		}
		take_objs(ls);

		for (auto& f : deltas) f();
		deltas.clear();
		for (auto &o : objs){
			o.checkpoint();
		}
		
	}

	template<typename T>
	T* get (TypedHandle<T> h, T* init){
		if (contains_obj(h.id()) ){
			assert(obj_matches(h));
			return new T(h.obj.curr);
		}
		else return init;
	}
};
