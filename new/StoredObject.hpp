#pragma once
#include "Backend.hpp"
#include "Instance.hpp"
#include <memory>

template<location l>
class Instance<l>::StoredBlob{
};

template<location l>
template<typename T> 
class Instance<l>::StoredObject {
private:
	std::unique_ptr<T> t;
public:
	StoredObject(std::unique_ptr<T> t):t(std::move(t)){}
	
};
