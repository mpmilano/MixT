#pragma once
#include <set>

template<typename T>
class SafeSet{
	std::set impl;
	std::mutex m;
	using lock = std::unique_lock<std::mutex>;
public:
	template<typename... Args>
	T& emplace(Args && ... args){
		lock l{m}; discard(l);
		return *impl.emplace(std::forward<Args>(args)...).first;
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

	void add(T t){
		lock l{m}; discard(l);
		impl.insert(t);
	}

	void remove(const T &t){
		lock l{m}; discard(l);
		impl.erase(t);
	}
	
};
