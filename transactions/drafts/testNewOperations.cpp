#include "NewOperations.hpp"


int main(){
	FakeStore fs;
	using T = int;
	using Hndl = typename FakeStore::FakeHandle<T>;
	using Op = OperationIdentifier<FakeStore::Increment>;
	constexpr Op opid{nullptr};
	Hndl hndl = fs.template newObject<T>();
	do_op_2<FakeStore::Increment>(hndl);
}
