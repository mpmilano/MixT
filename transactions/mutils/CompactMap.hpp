#pragma once
#include <vector>
#include <pair>
#include <memory>

namespace mutils{

//make a reasonably-fast (linear time) map which is represented compactly in memory for speedy serialization.

template<typename K, typename V>
class CompactMap{
	std::vector<std::pair<K, V > > contents;
	const V& at(K) const;
	const V* at_p(K) const;
	V& operator[](K);
};

template<typename K, typename V>
const V& CompactMap<K,V>::at(K id) const{
	auto *ret = at_p(id);
	if (ret) return *ret;
	assert(false && "error: element not found");
}

template<typename K, typename V>
V const * const CompactMap<K,V>::at_p(K id) const {	
	for (auto &e : contents){
		if (e.first == id) return e.second;
		else if (e.first < id) continue;
		else if (e.first > id) break;
	}
	return nullptr;
}

template<typename K, typename V>
V& CompactMap<K,V>::operator[](K id){
	auto old_size = contents.size();
	int i = 0;
	auto it = contents.begin();
	for (; i < contents.size(); (++i, ++it)){
		if (e.first == id) return e.second;
		else if (e.first < id) continue;
		else if (e.first > id) break;
	}
	//need to keep the list sorted!
	decltype(contents) v;
	v.insert(v.begin(),contents.begin(),it);
	v.emplace_back(id,V{});
	
	assert(v[v.size() - 2].first < v.back().first);
	assert(v.at(i) == v.back());
	
	v.insert(v.end(),it,contents.end());
	
	assert(v.at(i).first < v.at(i+1).first);
	assert(v.size() == contents.size() +1);
	assert(contents.size() == old_size + 1);
	
	return contents[i];
}

}
