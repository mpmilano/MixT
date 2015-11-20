#pragma once
#include <set>
#include <list>
#include <thread>
#include <mutex>

namespace mutils{

template<typename T>
void discard(const T&){}

template<typename T>
struct MonotoneSafeSet {
	std::list<T> impl;
	std::mutex m;
	using lock = std::unique_lock<std::mutex>;
	template<typename... Args>
	T& emplace(Args && ... args){
		lock l{m}; discard(l);
		impl.emplace_back(std::forward<Args>(args)...);
		return impl.back();
	}
	bool empty() const{
		return impl.empty();
	}

	auto size() const{
		return impl.size();
	}

	void add(T t){
		lock l{m}; discard(l);
		impl.push_back(t);
	}

	void remove(const T &t){
		lock l{m}; discard(l);
		impl.erase(std::find(impl.begin(), impl.end(),t));
	}
	
};

template<typename T>
struct SafeSet : MonotoneSafeSet<T>{

	struct EmptyException{};
	using lock = typename MonotoneSafeSet<T>::lock;
	T pop(){
		lock l{this->m}; discard(l);
		if (this->impl.size() == 0) throw EmptyException{};
		auto r = this->impl.front();
		this->impl.pop_front();
		return r;
	}
};

template<typename T>
struct SafeSet<T*> : MonotoneSafeSet<T*>{
	using lock = typename MonotoneSafeSet<T>::lock;
	T* pop(){
		lock l{this->m}; discard(l);
		if (this->impl.size() == 0) return nullptr;
		auto r = this->impl.front();
		this->impl.pop_front();
		return r;
	}	
};
}
