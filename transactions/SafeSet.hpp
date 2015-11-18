#pragma once
#include <set>
#include <list>
#include <thread>
#include <mutex>

template<typename T>
void discard(const T&){}

template<typename T>
struct SafeSet{
	std::list<T> impl;
	std::mutex m;
	using lock = std::unique_lock<std::mutex>;
	template<typename... Args>
	T& emplace(Args && ... args){
		lock l{m}; discard(l);
		impl.emplace_back(std::forward<Args>(args)...);
		return impl.back();
	}

	T pop(){
		lock l{m}; discard(l);
		auto r = impl.front();
		impl.pop_front();
		return r;
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
