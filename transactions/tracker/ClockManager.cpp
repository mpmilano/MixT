#include "Tracker.hpp"
#include "Tracker_private_declarations.hpp"

using namespace std;

namespace myria { namespace tracker {

		struct ClockManager{
			static ClockManager& inst(){
				static ClockManager inst;
				return inst;
			}
			
			Tracker::Clock clock;

		private:
			ClockManager(){
				
			}
			ClockManager(const ClockManager&) = delete;
			
		};

		void Tracker::updateClock(TrackingContext &tctx){
			auto &newc = ClockManager::inst().clock;
			assert(ends::prec(i->global_min,newc));
			i->global_min = newc;
			list<Name> to_remove;
			for (auto& p : i->tracking){
				if (ends::prec(p.second.first,newc)) to_remove.push_back(p.first);
				}
			for (auto &e : to_remove){
					tctx.i->tracking_erase.push_back(e);
			}
		}

	}}
