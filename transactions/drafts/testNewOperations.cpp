#include "NewOperations.hpp"


int main(){
	FakeStore fs;
	using T = int;
	typename FakeStore::FakeHandle<T> hndl = fs.template newObject<T>();
	do_op<FakeStore::Increment>(hndl).act();
}
