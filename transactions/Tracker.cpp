#include "Tracker.hpp"

Tracker::Tracker(){}

Tracker& Tracker::global_tracker(){
		static Tracker t;
		return t;
	}

