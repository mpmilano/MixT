#include "allocator.hpp"
#include <iostream>


struct child {
	allocated_ref<child> c;
	std::size_t payload;

	constexpr child():payload{0}{}
	constexpr child(child&& c)
		:c{std::move(c.c)},
		 payload{c.payload}{}

};

std::ostream& operator<<(std::ostream &o, const child& c){
	return o << "I'm a child! " << c.payload;
}

struct top{
	allocated_ref<child> c;

	constexpr top(){}
	constexpr top(top&& t):c{std::move(t.c)}{}	

};

constexpr Allocator<15,top,child> test_allocator
(Allocator<15,top,child> allocator = Allocator<15,top,child>{}){
	allocator.top.c = allocator.template allocate<child>();
	allocator.top.$(c).c = allocator.template allocate<child>();
	allocator.top.$(c).$(c).payload = 14;
	return allocator;
}

struct _int{
	int i;
	constexpr _int():i(0){}
};

constexpr int test_array(array<_int,25> arr = array<_int,25>{}){
	_int* temp{nullptr};
	auto ret = arr[0];
	temp = arr.ptr(0);
	return (*temp).i + ret.i;
}

int main() {

	constexpr Allocator<15,top,child> allocator{test_allocator()};
	constexpr auto payload = allocator.top.$(c).$(c).payload;
	std::cout << payload << std::endl;
	constexpr int arr = test_array();
	
	
	
}

//*/
