#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <thread>
#include "ctpl_stl.h"
#include "CooperativeCache.hpp"
#include "FutureFreePool.hpp"
#include "Ostreams.hpp"

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
			:active_behavior(beh),
			 tp(behavior_make_requests(beh) ? new ctpl::thread_pool(tp_size) : nullptr){}
		
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

#define fail_on_false(a...) {auto bawef = a; assert(bawef);}
		namespace {
		
			void respond_to_request(std::shared_ptr<std::mutex> m, std::shared_ptr<CooperativeCache::cache_t> cache, int socket){
				//std::cout << "Responding to request (" << cache << ").  Our contents are:" << std::endl;
				//for (auto& e : *cache){
					//std::cout << e.first << " --> " << e.second << std::endl;
				//}
				AtScopeEnd ase{[&](){close(socket);}}; discard(ase);
				Tracker::Nonce requested = 0;
				int n = read(socket,&requested,sizeof(Tracker::Nonce));
				

				if (n < 0) std::cerr << "ERROR reading from socket" << std::endl;
				assert(n >= 0);
				bool b = false;
				
				if (cache->count(requested) > 0){
					
					b = true;
					CooperativeCache::obj_bundle o;
					{
						CooperativeCache::CooperativeCache::lock l{*m};
						o = cache->at(requested);
					}
					for (auto &e : o){
						assert(e.third.data());
					}
					
					
#define care_write(x...) fail_on_false(write(socket,&(x),sizeof((x))) == sizeof((x)))
					care_write(b);
					int size = o.size();
					
					care_write(size)
					for (const Tracker::StampedObject &m : o){
						
						care_write(m.first);
						for (auto i : m.second){
							
							care_write(i);
						}
						int s = m.third.size();
						
						care_write(s);
						
						fail_on_false(write(socket,m.third.data(),s) == s);
					}
				}
				else {
					
					fail_on_false(write(socket,&b,sizeof(b)) >= 0);
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
				return tp->push([tomb,portno,this](int) -> CooperativeCache::obj_bundle {
						while (true)
							try {
								int sockfd;
								struct sockaddr_in serv_addr;
								struct hostent *server;
								
								sockfd = socket(AF_INET, SOCK_STREAM, 0);
								if (sockfd < 0){
									std::cerr << ("ERROR opening socket") << std::endl;
									throw NetException{};
								}
								//linger lingerStruct{0,0};
								//setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (void *)&lingerStruct, sizeof(lingerStruct));
								
								AtScopeEnd ase{[&](){close(sockfd);}};
								discard(ase);
								server = gethostbyaddr(&tomb.ip_addr,sizeof(tomb.ip_addr),AF_INET);
								bzero((char *) &serv_addr, sizeof(serv_addr));
								serv_addr.sin_family = AF_INET;
								bcopy((char *)server->h_addr,
									  (char *)&serv_addr.sin_addr.s_addr,
									  server->h_length);
								serv_addr.sin_port = htons(portno);
								if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
									std::cerr << ("ERROR connecting");
									throw NetException{};
								}
								
								//UGH there's a lot of boilerplate when you do networking.
#define care_read_s(s,x...)												\
								{										\
									int n = read(sockfd,&(x),s);		\
									std::stringstream err;				\
									if (n < 0) {						\
										std::stringstream err;			\
										err << "expected " << s << " bytes, received " << n << " accompanying error: " << std::strerror(errno); \
										throw ProtocolException(err.str());	\
									}									\
									while (n < s) {						\
										/*std::cout << "WARNING: only got " << n << " bytes, expected " << s << " bytes" <<std::endl; \
										  std::cout << "value read so far: " << (x) << std::endl; */ \
										int k = read(sockfd,((char*) &(x)) + n,s-n); \
										if (k <= 0) {					\
											std::stringstream err;		\
											err << "expected " << s << " bytes, received " << n << " accompanying error: " << std::strerror(errno); \
											throw ProtocolException(err.str());	\
										}								\
										n += k;							\
									}}
								
#define care_read(x...) {int s = sizeof(x); care_read_s(s,x)}
								
								
#define care_assert(x...) if (!(x)) throw ProtocolException(std::string("Assert failed: ") + #x);
								
								auto tname = tomb.name();
								
								fail_on_false(write(sockfd,&tname,sizeof(tname)) >= 0);
								
								bool success = false;
								int num_messages = 0;
								care_read(success);
								
								if (!success) throw CacheMiss{};
								care_read(num_messages);
								
								care_assert(num_messages > 0);
								obj_bundle ret;
								for (int i = 0; i < num_messages; ++i){
									Name name;
									care_read(name);
									
									Tracker::Clock clock;
									for (int j = 0; j < clock.size(); ++j){
										care_read(clock[j]);
										
									}
									int obj_size;
									care_read(obj_size);
									
									std::vector<char> obj(obj_size,0);
									assert(obj.size() == obj_size);
									//char bytes[obj_size];
									care_read_s(obj_size,*obj.data());
									
									ret.emplace_back(name,clock,obj);
								}
								{
									lock l{*m};
									track_with_eviction(tomb.nonce,ret);
								}
								//std::cout << "retrieved " << tomb.nonce << " via Cache" << std::endl;
								for (auto &e : ret){
									assert(e.third.data());
								}
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
							int sockfd;
							socklen_t clilen;
							struct sockaddr_in serv_addr, cli_addr;
							sockfd = socket(AF_INET, SOCK_STREAM, 0);
							//linger lingerStruct{0,0};
							//setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (void *)&lingerStruct, sizeof(lingerStruct));
							if (sockfd < 0){
								std::cerr << "ERROR opening socket" << std::endl;
								return;
							}
							bzero((char *) &serv_addr, sizeof(serv_addr));
							serv_addr.sin_family = AF_INET;
							serv_addr.sin_addr.s_addr = INADDR_ANY;
							serv_addr.sin_port = htons(portno);
							if (bind(sockfd, (struct sockaddr *) &serv_addr,
									 sizeof(serv_addr)) < 0){
								std::cerr << "ERROR on binding" << std::endl;
								return;
							}
							fail_on_false(listen(sockfd,50) == 0);
							clilen = sizeof(cli_addr);
							AtScopeEnd ase{[&](){close(sockfd);}};
							constexpr int tp_size2 = tp_size;
							FutureFreePool pool{tp_size2};
							while (true) {
								int newsockfd = accept(sockfd,
													   (struct sockaddr *) &cli_addr,
													   &clilen);
								if (newsockfd < 0){
									std::cerr << "ERROR on accept: "
											  << std::strerror(errno)
											  << std::endl;
									continue;
								}
								//fork off a new thread to handle the request.
								pool.launch([=](int){respond_to_request(m,cache,newsockfd);});
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

		std::vector<char> const * const CooperativeCache::find(const obj_bundle& b,const Name& n, const Tracker::Clock &version){
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
