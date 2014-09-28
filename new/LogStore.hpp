#pragma once
#include "Backend.hpp"
#include "Instance.hpp"
#include <memory>
#include <functional>
#include <list>

template<location l>
template<Instance<l>::LSWhen when>
class Instance<l>::LogStore{
private:
	std::function<std::unique_ptr<GenericHandle> (GenericHandle::HandleID) > idmap = 
		[](GenericHandle::HandleID){ return std::unique_ptr<GenericHandle>(nullptr); };

	std::list<std::function<void ()> > deltas;

	std::list<std::unique_ptr<StoredBlob> > objs;
	
	
	template<typename T>
	T freshen_handles(T t){
		static_assert(is_handle<T>, "can't freshen non-handles!");
		auto ptr = idmap(t.id());
		assert(ptr->is_type(T* (nullptr)));
		return *((T*) ptr.get());
	}

	template<typename... Args>
	bool containsHandle(GenericHandle::HandleID hid, Args... h){
		static_assert(sizeof...(Args) == 0,"whoops");
		return 0;
	}

	template<typename... Args>
	bool containsHandle(GenericHandle::HandleID hid, GenericHandle& h, Args... h){
		return h.id() == hid || containsHandle(hid,h...);
	}

	template<typename T_, typename... Args>
	auto findHandle(GenericHandle::HandleID hid, TypedHandle<T_,l2,a> h, Args... hrest){
		return std::unique_ptr<GenericHandle>(
			hid == h.id() ? new TypedHandle<T_>(h) : findHandle(hid,hrest,h));
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
		static_assert(forall<is_handle,Args...>, "Params must be handles!");
		auto newmap = [idmap, params...](GenericHandle::HandleID id){
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
		auto idmap = [&](GenericHandle::HandleID id){
			auto tmp = to.idmap(id);
			if (tmp) return tmp;
			else assert(false && "TODO");
		};
		this->idmap = idmap;

		for (auto& f : deltas) f();
		deltas.clear();
		idmap = [](GenericHandle::HandleID){ return std::unique_ptr<GenericHandle>(nullptr); };
	}

	template<location l2>
	void replaceWith(Instance<l2>::LogStore<LSWhen::immediate> &from){
		static_assert(l2 != l, "This was intended for replacing local state with remote state.");
		//TODO body here
	}
};
