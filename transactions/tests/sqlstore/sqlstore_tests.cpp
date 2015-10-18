#undef STORE_LIST
#define STORE_LIST SQLStore

#include "SQLStore.hpp"
#include "FinalHeader.hpp"

namespace {

	SQLStore& store = SQLStore::inst();
	bool creation_and_existence(){
		auto h1 = store.newObject<HandleAccess::all>(1,12);
		assert(store.exists(1));
		auto h2 = store.existingObject<HandleAccess::all,int>(1);
		assert(h1.get() == h2.get());
		store.remove(1);
		assert(!h1.isValid());
		assert(!h2.isValid());
		return true;
	}

	bool updating_values(){
		auto h1 = store.newObject<HandleAccess::all>(1,12);
		assert(h1.get() == 12);
		h1.put(54);
		assert(h1.get() == 54);
		store.remove(1);
		return true;
	}

	bool storing_itself(){
		auto h1 = store.newObject<HandleAccess::all>(1,12);
		assert(h1.get() == 12);
		auto h2 = store.newObject<HandleAccess::all>(2,h1);
		assert(h2.get().get() == 12);
		h1.put(34);
		assert(h2.get().get() == 34);
		std::cout << h2.get().get() << std::endl;
		store.remove(1);
		store.remove(2);
		return true;
	}

}

int main(){
	assert(creation_and_existence());
	assert(updating_values());
	assert(storing_itself());
}
