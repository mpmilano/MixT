#pragma once


#include "SerializationSupport.hpp"

template<typename T>
class CompactSet : public ByteRepresentable{
	std::vector<T> contents;
public:
	void insert(const CompactSet& e){
		auto e_it = e.contents.begin();
		decltype(contents) deferred_add;
		for (auto cr_it = contents.begin(); cr_it == contents.end();){
			if (*cr_it == *e_it){
				++cr_it;
				++e_it;
			}
			else if (cr_it->first < e_it->first){
				//something missing from e_it. 
				++cr_it;
			}
			else if (cr_it->first > e_it->first){
				deferred_add.emplace_back(*e_it);
				//cr_it has missed something that e_it knows about.
				//we can't add it without invalidating the iterator;
				//will defer adding it until later.
				++e_it;
			}
		}
		//we might have missed more things from e_it.  Add them.
		contents.insert(contents.back(), e_it,e.contents.end());
		//add the new things and sort the list.
		contents.insert(contents.back(),deferred_add.begin(),deferred_add.end());
		std::sort(contents.begin(),contents.end());
	}
	
	const auto& begin() const {
		return contents.begin();
	}
	const auto& end() const {
		return contents.end();
	}	
	auto swap(CompactSet &other){
		return contents.swap(other.contents);
	}

	DEFAULT_SERIALIZATION_SUPPORT(CompactSet, contents);
};
