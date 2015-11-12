

#include "SQLStore.hpp"
#include "FinalHeader.hpp"

namespace {

	SQLStore<Level::strong>& store = SQLStore<Level::strong>::inst(0);
	int start;
	
	bool creation_and_existence(){
		auto h1 = store.newObject<HandleAccess::all>(1 + start,12);
		assert(store.exists(1 + start));
		auto h2 = store.existingObject<HandleAccess::all,int>(1 + start);
		assert(h1.get() == h2.get());
		store.remove(1 + start);
		assert(!h1.isValid());
		assert(!h2.isValid());
		return true;
	}

	bool updating_values(){
		auto h1 = store.newObject<HandleAccess::all>(1 + start,12);
		assert(h1.get() == 12);
		h1.put(54);
		assert(h1.get() == 54);
                store.remove(1 + start);
		return true;
	}

	bool storing_itself(){
		auto h1 = store.newObject<HandleAccess::all>(1 + start,12);
		assert(h1.get() == 12);
		auto h2 = store.newObject<HandleAccess::all>(2 + start,h1);
		assert(h2.get().get() == 12);
		h1.put(34);
		assert(h2.get().get() == 34);
		std::cout << h2.get().get() << std::endl;
		store.remove(1 + start);
		store.remove(2 + start);
		return true;
	}

	template<typename Causal>
	void causal_basic(Causal& st){
                st.remove(1 + start);
		auto ch = st.template newObject<HandleAccess::all>(1 + start, 5);
		ch.put(6);
		assert(ch.get() == 6);
                st.remove(1 + start);
	}

}

int main(){
	timespec ts;
	clock_gettime(CLOCK_REALTIME,&ts);
	srand(ts.tv_nsec);
	start = rand();
	auto &st = SQLStore<Level::causal>::inst(0); //tracker needs registration
	assert(creation_and_existence());
	assert(updating_values());
	assert(storing_itself());
	causal_basic(st);
	//add a causal object to the tracking set
	Tracker &t = Tracker::global_tracker();
	t.assert_nonempty_tracking();
	assert(creation_and_existence());
	assert(updating_values());
	assert(storing_itself());
}
