#include "Tracker.hpp"
#include "Tracker_private_declarations.hpp"
#include "ServerSocket.hpp"
#include <thread>

using namespace std;
using namespace mutils;

namespace myria { namespace tracker {

		struct ClockManager{
			static ClockManager& inst(){
				static ClockManager inst;
				return inst;
			}

			volatile typename Tracker::Clock::value_type clock
			[Tracker::Clock{}.max_size()];

			ClockManager(const ClockManager&) = delete;
		private:
			AcceptConnectionLoop loop{
				[&](bool&, Socket sock){
					try{
						while (sock.valid()){
							Tracker::Clock tmpclock;
							sock.receive(tmpclock);
							for (std::decay_t<decltype(tmpclock.size())> i = 0; i < tmpclock.size(); ++i){
								clock[i] = tmpclock[i];
							}
						}
					}
					catch (const ProtocolException&){
					}
				}};
			
			ClockManager(){
				std::thread{[loop = this->loop]() mutable {
						try{
							loop.loop_until_dead(Tracker::clockport);
						}
						catch(mutils::SocketException&){
                                              //looks like we're flying without the clock today.
						}
					}}.detach();
			}
			~ClockManager(){
				*loop.alive = false;
			}
			
		};

                void Tracker::updateClock(){
			Tracker::Clock newc;
			for (std::decay_t<decltype(newc.size())> i = 0; i < newc.size(); ++i){
				auto &inst = ClockManager::inst();
				if (inst.clock[i] == -1) {
					--i;
					continue;
				}
				volatile int &tmpi = inst.clock[i];
				newc[i] = tmpi;
			}
			assert(ends::prec(i->global_min,newc));
			i->global_min = newc;
			list<Name> to_remove;
			for (auto& p : i->tracking){
				if (ends::prec(p.second.first,newc)) to_remove.push_back(p.first);
				}
			for (auto &e : to_remove){
                                        i->tracking.erase(e);
			}
		}

	}}
