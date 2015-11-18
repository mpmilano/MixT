#pragma once
#include <set>
#include <thread>
#include <mutex>

template<typename T>
void discard(const T&){}

template<typename T>
struct SafeSet{
	std::set<T> impl;
	std::mutex m;
	using lock = std::unique_lock<std::mutex>;
	template<typename... Args>
	T& emplace(Args && ... args){
		lock l{m}; discard(l);
		std::pair<typename decltype(impl)::iterator,bool> ret = impl.emplace(std::forward<Args>(args)...);
		//I know, it's evil.
		return const_cast<T&>(*ret.first);
	}

	T pop(){
		lock l{m}; discard(l);
		auto r = *impl.begin();
		impl.erase(impl.begin());
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
		impl.insert(t);
	}

	void remove(const T &t){
		lock l{m}; discard(l);
		impl.erase(t);
	}
	
};
