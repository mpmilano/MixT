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
template<typename Instance<l>::LSWhen when>
class Instance<l>::LogStore{
private:

	static std::unique_ptr<GenericHandle> init_idmap(typename GenericHandle::HandleID) { 
		return std::unique_ptr<GenericHandle>(nullptr); 
	}

	std::function< decltype(init_idmap) > idmap = init_idmap;
		

	std::list<std::function<void ()> > deltas;

	std::list<std::unique_ptr<StoredBlob> > objs;
	
	
	template<typename T>
	T freshen_handles(T t){
		static_assert(is_handle<T>::value, "can't freshen non-handles!");
		auto ptr = idmap(t.id());
		assert(dynamic_cast<T*>(ptr));
		return *((T*) ptr.get());
	}

	template<typename... Args>
	static bool containsHandle(typename GenericHandle::HandleID, Args... ){
		static_assert(sizeof...(Args) == 0,"whoops");
		return 0;
	}

	template<typename... Args>
	static bool containsHandle(typename GenericHandle::HandleID hid, GenericHandle& h, Args... a){
		return h.id() == hid || containsHandle(hid,a...);
	}

	template<typename T_, typename... Args>
	static auto findHandle(typename GenericHandle::HandleID hid, TypedHandle<T_> h, Args... hrest){
		return std::unique_ptr<GenericHandle>(
			hid == h.id() ? new TypedHandle<T_>(h) : findHandle(hid,hrest...,h));
		//will run forever if we can't find what we're looking for.
	}

	
public:

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
	
	template<typename... Args>
	void add(std::function<void (Args...)> f, Args... params){
		static_assert(forall(is_handle<Args>::value...), "Params must be handles!");
		auto idmap = this->idmap;
		auto newmap = [idmap, params...](typename GenericHandle::HandleID id){
			if (containsHandle(id, params...)){
				return findHandle(id, params...);
			}
			else return idmap(id);
		};
		this->idmap = newmap;
		std::function<void ()> update = [this,f,params...]() {
			f(freshen_handles(params)...);
		};
		if (when == LSWhen::immediate) update();
		else deltas.push_back(update);
	}
	void flush(LogStore<LSWhen::immediate> &to){
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
};
