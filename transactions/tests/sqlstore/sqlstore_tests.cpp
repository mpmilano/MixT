#include "SQLStore.hpp"

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

}

int main(){
	assert(creation_and_existence());
}
