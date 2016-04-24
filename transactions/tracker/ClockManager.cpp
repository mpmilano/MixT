#include "Tracker.hpp"
#include "Tracker_private_declarations.hpp"
#include "ServerSocket.hpp"

using namespace std;

namespace myria { namespace tracker {

		struct ClockManager{
			static ClockManager& inst(){
				static ClockManager inst;
				return inst;
			}

			volatile typename Tracker::Clock::value_type clock
			[Tracker::Clock{}.max_size()];

		private:
			ClockManager(){
				using namespace mutils;
				ServerSocket ss{Tracker::clockport,
						[&](Socket sock){
						try{
							while (sock.valid()){
								Tracker::Clock tmpclock;
								sock.receive(tmpclock);
								for (int i = 0; i < tmpclock.size(); ++i){
									clock[i] = tmpclock[i];
								}
							}
						}
						catch (const ProtocolException&){
						}
					}
						};
			}
			ClockManager(const ClockManager&) = delete;
			
		};

		void Tracker::updateClock(TrackingContext &tctx){
			Tracker::Clock newc;
			for (int i = 0; i < newc.size(); ++i){
				volatile int &tmpi = ClockManager::inst().clock[i];
				newc[i] = tmpi;
			}
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
