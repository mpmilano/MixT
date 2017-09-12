#include "pgsql/SQLStore.hpp"
#include "FinalHeader.hpp"
#include "tracker/Tracker.hpp"


int main(){
	Tracker& t = Tracker::global_tracker();
	t.tick();
}
