//what goes here: how to do an operation.


template<typename Operation, typename Handle, typename... Args>
auto op(Handle , Args... ){
	static_assert(is_handle<Handle>::value,"First argument must be a Handle here.");
	return nullptr;
}

