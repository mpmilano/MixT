#include "FileStore.hpp"
#include "SQLStore.hpp"
#include "FinalHeader.hpp"
#include "Tracker.hpp"


int main(){
	Tracker& t = Tracker::global_tracker();
	t.tick();
}
