#include "GlobalPool.hpp"
#include "CooperativeCache.hpp"
#include "FutureFreePool.hpp"
#include "Ostreams.hpp"
#include "Socket.hpp"
#include "ServerSocket.hpp"

using namespace mutils;

namespace myria { namespace tracker {

		namespace{
			bool behavior_accept_requests(CacheBehaviors beh){
				return beh == CacheBehaviors::onlyaccept || beh == CacheBehaviors::full;
			}

			bool behavior_make_requests(CacheBehaviors beh){
				return beh == CacheBehaviors::onlymake || beh == CacheBehaviors::full;
			}
		}

		struct hostent * extract_ip(Tracker::Nonce n);

		void CooperativeCache::track_with_eviction(Tracker::Nonce n, const obj_bundle &o){
			if (order.size() > max_size){
				auto elim = order.front();
				order.pop_front();
				cache->erase(elim);
			}
			order.push_back(n);
			(*cache)[n] = o;
		}

		CooperativeCache::CooperativeCache(CacheBehaviors beh)
			:active_behavior(beh){}
		
		void CooperativeCache::insert(Tracker::Nonce n, const std::map<Name,std::pair<Tracker::Clock, std::vector<char> > > &map){
			obj_bundle new_tracking;
			for (auto &p : map){
				assert(p.second.second.data());
				new_tracking.emplace_back(p.first,p.second.first,p.second.second);
			}
			assert([&]() -> bool{
					for (auto &e : new_tracking){
						assert(e.third.data());
					}
					return true;
				}());
			{
				lock l{*m};
				track_with_eviction(n,new_tracking);
			}
		}

/* 
   protocol order:
   - Nonce indicating requested element
   - boolean indicating success
   - int indicating number of messages
   - name
   - clock (component-wise)
   - size of obj
   - obj
*/

#ifndef NDEBUG
#define fail_on_false(a...) {auto bawef = a; assert(bawef);}
#else
#define fail_on_false(...) ;
#endif
		
		namespace {
		
			void respond_to_request(std::shared_ptr<std::mutex> m, std::shared_ptr<CooperativeCache::cache_t> cache, Socket socket){
				//std::cout << "Responding to request (" << cache << ").  Our contents are:" << std::endl;
				//for (auto& e : *cache){
					//std::cout << e.first << " --> " << e.second << std::endl;
				//}
				Tracker::Nonce requested = 0;
				socket.receive(requested);
				
				if (cache->count(requested) > 0){
					CooperativeCache::obj_bundle o;
					{
						CooperativeCache::CooperativeCache::lock l{*m};
						o = cache->at(requested);
					}
					assert([&]{
							for (auto &e : o){
								assert(e.third.data());
							}
							return true;
						}());

					
					socket.send(true);
					socket.send(o.size());
					for (const Tracker::StampedObject &m : o){

						socket.send(m.first);
						for (auto i : m.second){
							socket.send(i);
						}
						int s = m.third.size();

						socket.send(s);
						socket.send(s,m.third.data());
					}
				}
				else {
					socket.send(false);
				}
			}
		}

		bool CooperativeCache::contains(const Tracker::Tombstone& tomb) const{
			return (cache->count(tomb.nonce) > 0);
		}

		bool CooperativeCache::contains(const Tracker::Nonce& tomb) const{
			return (cache->count(tomb) > 0);
		}

		std::future<CooperativeCache::obj_bundle> CooperativeCache::get(const Tracker::Tombstone &tomb){

			const int portno = tomb.portno;
			//std::cout << "Requesting object " << tomb.name() << " (" << cache << ")" << std::endl;
			
			
			{
				lock l{*m};
				if (cache->count(tomb.nonce) > 0) {
					//std::cout << "retrieved " << tomb.nonce << " via Cache" << std::endl;
					return std::async(std::launch::deferred, [r = cache->at(tomb.nonce)](){return r;});
				}
			}
			if (behavior_make_requests(active_behavior)){
			  return std::async(std::launch::async,[tomb,portno,this]() -> CooperativeCache::obj_bundle {
						while (true)
							try {
								auto tname = tomb.name();

								Socket remote = Socket::connect(tomb.ip_addr,portno);
								remote.send(tname);

								bool success;
								remote.receive(success);
								if (!success) throw CacheMiss{};
								int num_messages;
								remote.receive(num_messages);
								
								assert(num_messages > 0);
								obj_bundle ret;
								for (int i = 0; i < num_messages; ++i){
									Name name;
									remote.receive(name);
									Tracker::Clock clock;
									for (std::decay_t<decltype(clock.size())> j = 0; j < clock.size(); ++j){
										remote.receive(clock[j]);
									}
									int obj_size;
									remote.receive(obj_size);

									std::vector<char> obj(obj_size,0);
									//char bytes[obj_size];
									remote.receive(obj_size,obj.data());
									assert(obj.size() == (unsigned int) obj_size);
									
									ret.emplace_back(name,clock,obj);
								}
								{
									lock l{*m};
									track_with_eviction(tomb.nonce,ret);
								}
								//std::cout << "retrieved " << tomb.nonce << " via Cache" << std::endl;
#ifndef NDEBUG
								for (auto &e : ret){
									assert(e.third.data());
								}
#endif
								return ret;
							}
							catch (const ProtocolException &e){
								std::cout << "Protocol exception: " << e.what() << ", trying again" << std::endl;
								continue;
							}
							});
					}
				else return std::async(std::launch::deferred,[]() -> CooperativeCache::obj_bundle {
					std::cerr << "ERROR: CACHE DISABLED" << std::endl;
					throw CacheMiss{};});
			}


		void CooperativeCache::listen_on(int portno){
			//std::cout << "listening on " << portno << std::endl;
			//fork off this thread to listen for and respond to cooperative cache requests
			if (behavior_accept_requests(active_behavior)){
				auto cache = this->cache;
				auto m = this->m;
				std::thread([m,portno,cache](){
						try {
							AtScopeEnd ase2{[](){std::cout << "listening done; cache closed" << std::endl;}};
							ServerSocket server(portno);
							
							while (true) {
								//fork off a new thread to handle the request.
							  std::async(std::launch::async,[m,cache,newsockfd = server.receive()]() mutable {
										if (newsockfd.valid())
											respond_to_request(m,cache,std::move(newsockfd));});
							}
						}
						catch(const std::exception & e){
							std::cout << "Cache accept thread failure: " << e.what() << std::endl;
						}
						catch(...){
							std::cout << "Cache accept thread failure!" << std::endl;
						}
					}).detach();
			}
		}	

		std::vector<char> const *  CooperativeCache::find(const obj_bundle& b,const Name& n, const Tracker::Clock &version){
			(void) version;
			for (auto &e : b){
				assert(e.third.data());
				const Name& ename = e.first;
				if (n == ename){
					assert(ends::prec(version, e.second) || [&](){
							if (ends::prec(e.second, version)){
							std::cout << "Error: found version predates known version" << std::endl;
							std::cout << "Found version: " << e.second << std::endl;
							std::cout << "Known version: " << version << std::endl;
						}
						return !ends::prec(e.second, version);
						}());
					return &e.third;
				}
			}
			return nullptr;
		}					
	}}
