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

namespace{
	std::mutex& debugging_lock(){
		static std::mutex m;
		return m;
	}
}

using namespace mutils;

namespace myria { namespace tracker { 

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

		CooperativeCache::CooperativeCache(){}
		void CooperativeCache::insert(Tracker::Nonce n, const std::map<Name,std::pair<Tracker::Clock, std::vector<char> > > &map){
			obj_bundle new_tracking;
			for (auto &p : map){
				new_tracking.emplace_back(p.first,p.second.first,p.second.second);
			}
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
				AtScopeEnd ase{[&](){close(socket);}}; discard(ase);
				Tracker::Nonce requested = 0;
				int n = read(socket,&requested,sizeof(Tracker::Nonce));
				{
					CooperativeCache::lock l{debugging_lock()}; 
				std::cout << "responding to request: hopefully just read a request Nonce" << std::endl;
				}

				if (n < 0) std::cerr << "ERROR reading from socket" << std::endl;
				assert(n >= 0);
				bool b = false;
				{
					CooperativeCache::lock l{debugging_lock()}; 
				std::cout << "requested: " << requested << std::endl;
				}
				if (cache->count(requested) > 0){
					{
						CooperativeCache::lock l{debugging_lock()}; 
					std::cout << "we have it!" << std::endl;
					}
					b = true;
					CooperativeCache::obj_bundle o;
					{
						CooperativeCache::CooperativeCache::lock l{*m};
						o = cache->at(requested);
					}
					{
						CooperativeCache::lock l{debugging_lock()}; 
						std::cout << "here is what we found: " << o << std::endl;
					}
					{
						CooperativeCache::lock l{debugging_lock()}; 
						std::cout << "writing boolean: " << b << " of size " << sizeof(b) << std::endl;
					}
#define care_write(x...) fail_on_false(write(socket,&(x),sizeof((x))) == sizeof((x)))
					care_write(b);
					int size = o.size();
					{
						CooperativeCache::lock l{debugging_lock()}; 
						std::cout << "indicating number of entries which match: " << size << " of size " << sizeof(size) << std::endl;
					}
					care_write(size)
					for (const Tracker::StampedObject &m : o){
						{
							CooperativeCache::lock l{debugging_lock()};
							std::cout << "writing name: " << m.first << " of size " << sizeof(m.first) << std::endl;
						}
						care_write(m.first);
						for (auto i : m.second){
							{
								CooperativeCache::lock l{debugging_lock()}; 
								std::cout << "writing clock entry: " << i << " of size " << sizeof(i) << std::endl;
							}
							care_write(i);
						}
						int s = m.third.size();
						{
							CooperativeCache::lock l{debugging_lock()}; 
							std::cout << "writing num bytes in object " << s << " of size " << sizeof(s) << std::endl;
						}
						care_write(s);
						{
							CooperativeCache::lock l{debugging_lock()}; 
							std::cout << "writing object" << m.third << std::endl;
						}
						fail_on_false(write(socket,m.third.data(),s) == s);
					}
				}
				else {
					{
						CooperativeCache::lock l{debugging_lock()}; 
					std::cout << "we don't have it, apologizing" << std::endl;
					}
					fail_on_false(write(socket,&b,sizeof(b)) >= 0);
				}
			}
		}
		
		std::future<CooperativeCache::obj_bundle> CooperativeCache::get(const Tracker::Tombstone &tomb, int portno){
			{
				lock l{*m};
				if (cache->count(tomb.nonce) > 0) {
					return std::async(std::launch::deferred, [r = cache->at(tomb.nonce)](){return r;});
				}
			}
#ifdef MAKE_CACHE_REQUESTS 
			return tp.push([tomb,portno,this](int) -> CooperativeCache::obj_bundle {
					int sockfd;
					struct sockaddr_in serv_addr;
					struct hostent *server;
					
					sockfd = socket(AF_INET, SOCK_STREAM, 0);
					if (sockfd < 0){
						std::cerr << ("ERROR opening socket") << std::endl;
						throw NetException{};
					}
					linger lingerStruct{0,0};
					setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (void *)&lingerStruct, sizeof(lingerStruct));
					
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
					{													\
						int n = read(sockfd,&(x),s);					\
						std::stringstream err;							\
						if (n < 0) {									\
							std::stringstream err;						\
							err << "expected " << s << " bytes, received " << n << " accompanying error: " << std::strerror(errno); \
							throw ProtocolException(err.str());			\
						}												\
						while (n < s) {									\
							std::cout << "WARNING: only got " << n << " bytes, expected " << s << " bytes" <<std::endl; \
							std::cout << "value read so far: " << (x) << std::endl;	\
							int k = read(sockfd,((char*) &(x)) + n,s-n); \
							if (k <= 0) {								\
							std::stringstream err;						\
							err << "expected " << s << " bytes, received " << n << " accompanying error: " << std::strerror(errno); \
							throw ProtocolException(err.str());			\
							}											\
							n += k;										\
						}}
					
#define care_read(x...) {int s = sizeof(x); care_read_s(s,x)}
					
					
#define care_assert(x...) if (!(x)) throw ProtocolException(std::string("Assert failed: ") + #x);
					
					auto tname = tomb.name();
					{
						lock l{debugging_lock()}; 
						std::cout << "Connection established: writing nonce " << tname << " of size " << sizeof(tname) << std::endl;
					}
					fail_on_false(write(sockfd,&tname,sizeof(tname)) >= 0);
					
					bool success = false;
					int num_messages = 0;
					care_read(success);
					{
						lock l{debugging_lock()}; 
						std::cout << "hopefully just read to read a boolean: " << success << " of size " << sizeof(success) << std::endl;
					}
					care_assert(success);
					care_read(num_messages);
					{
						lock l{debugging_lock()}; 
						std::cout << "hopefully just read number of objects: " << num_messages << " of size " << sizeof(num_messages) << std::endl;
					}
					care_assert(num_messages > 0);
					obj_bundle ret;
					for (int i = 0; i < num_messages; ++i){
						Name name;
						care_read(name);
						{
							lock l{debugging_lock()}; 
							std::cout << "hopefully just read name: " << name << " of size " << sizeof(name) << std::endl;
						}
						Tracker::Clock clock;
						for (int j = 0; j < clock.size(); ++j){
							care_read(clock[j]);
							{
								lock l{debugging_lock()}; 
								std::cout << "hopefully just read part of a clock: " << clock[j] << " of size " << sizeof(clock[j]) << std::endl;
							}
						}
						int obj_size;
						care_read(obj_size);
						{
							lock l{debugging_lock()}; 
							std::cout << "hopefully just read object size: " << obj_size << " of size " << sizeof(obj_size) << std::endl;
						}
						std::vector<char> obj(obj_size,0);
						assert(obj.size() == obj_size);
						//char bytes[obj_size];
						care_read_s(obj_size,*obj.data());
						{
							lock l{debugging_lock()}; 
							//std::cout << "grabbed some object bytes: " << bytes << std::endl;
							std::cout << "bytes in a vector : " << obj << std::endl;
						}
						ret.emplace_back(name,clock,obj);
					}
					{
						lock l{*m};
						track_with_eviction(tomb.nonce,ret);
					}
					return ret;
				});
			#else
			return std::async(std::launch::deferred,[]() -> CooperativeCache::obj_bundle {assert(false && "cache is inactive!");});
			#endif
		}


		void CooperativeCache::listen_on(int portno){
			std::cout << "listening on " << portno << std::endl;
			//fork off this thread to listen for and respond to cooperative cache requests
			#ifdef ACCEPT_CACHE_REQUESTS
			auto cache = this->cache;
			auto m = this->m;
			std::thread([m,portno,cache](){
					try {
						AtScopeEnd ase2{[](){std::cout << "listening done; cache closed" << std::endl;}};
						int sockfd;
						socklen_t clilen;
						struct sockaddr_in serv_addr, cli_addr;
						sockfd = socket(AF_INET, SOCK_STREAM, 0);
						linger lingerStruct{0,0};
						setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (void *)&lingerStruct, sizeof(lingerStruct));
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
								std::cerr << "ERROR on accept" << std::endl;
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
			#endif
		}


		std::vector<char> const * const CooperativeCache::find(const obj_bundle& b,const Name& n, const Tracker::Clock &version){
			for (auto &e : b){
				const Name& ename = e.first;
				if (n == ename){
					assert(!ends::prec(e.second, version));
					return &e.third;
				}
			}
			return nullptr;
		}					
	}}
