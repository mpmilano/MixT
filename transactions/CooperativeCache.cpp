//move this into a source file eventually
#include "CooperativeCache.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <thread>

struct hostent * extract_ip(Tracker::Nonce n);

CooperativeCache::CooperativeCache(){}
void CooperativeCache::insert(Tracker::Nonce n, const std::map<int,std::pair<Tracker::Clock, std::vector<char> > > &map){
	obj_bundle new_tracking;
	for (auto &p : map){
		new_tracking.emplace_back(p.first,p.second.first,p.second.second);
	}
	{
		lock l{m};
		cache[n] = new_tracking;
	}
}

/* 
protocol order:
 - boolean indicating success
 - int indicating number of messages
    - name
    - clock (component-wise)
    - size of obj
    - obj
 */

void CooperativeCache::respond_to_request(int socket){
	AtScopeEnd ase{[&](){close(socket);}}; discard(ase);
	Tracker::Nonce requested = 0;
	int n = read(socket,&requested,sizeof(Tracker::Nonce));
	if (n < 0) std::cerr << "ERROR reading from socket" << std::endl;
	assert(n >= 0);
	bool b = false;
	if (cache.count(requested) > 0){
		b = true;
		obj_bundle o;
		{
			lock l{m};
			o = cache.at(requested);
		}
		assert(write(socket,&b,sizeof(b)) >= 0);
		int size = o.size();
		assert(write(socket,&(size),sizeof(&(size))) >= 0);
		for (auto &m : o){
			assert(write(socket,&(m.first),sizeof(m.first)) >= 0);
			for (auto i : m.second){
				assert(write(socket,&i,sizeof(i)) >= 0);
			}
			auto s = m.third.size();
			assert(write(socket,&(s),sizeof(s)) >= 0);
			assert(write(socket,m.third.data(),s) >= 0);
		}
	}
	else assert(write(socket,&b,sizeof(b)) >= 0);
}

std::unique_ptr<CooperativeCache::obj_bundle> CooperativeCache::get(Tracker::Nonce nonce, int portno){
	{
		lock l{m};
		if (cache.count(nonce) > 0) return heap_copy(cache.at(nonce));
	}
	std::unique_ptr<obj_bundle> nothingness{nullptr};
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		std::cerr << ("ERROR opening socket") << std::endl;
		return nothingness;
	}
	
	AtScopeEnd ase{[&](){close(sockfd);}};
	discard(ase);
	server = extract_ip(nonce);
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		  (char *)&serv_addr.sin_addr.s_addr,
		  server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		std::cerr << ("ERROR connecting");
		return nothingness;
	}

	//UGH there's a lot of boilerplate when you do networking.
#define care_read_s(s,x...)								\
	{													\
		int n = read(sockfd,&(x),s);					\
		if (n < 0) return nothingness;					\
		while (n < s) {									\
			int k = read(sockfd,((char*) &(x)) + n,s-n);	\
			if (k <= 0) return nothingness;				\
			n += k;										\
		}}
	
#define care_read(x...) {int s = sizeof(x); care_read_s(s,x)}


#define care_assert(x...) if (!(x)) return nothingness;
	try {
		bool success = false;
		int num_messages = 0;
		care_read(success);
		care_assert(success);
		care_read(num_messages);
		care_assert(num_messages > 0);
		std::unique_ptr<obj_bundle> ret{new obj_bundle{}};
		for (int i = 0; i < num_messages; ++i){
			int name;
			care_read(name);
			Tracker::Clock clock;
			for (int j = 0; j < clock.size(); ++j){
				care_read(clock[j]);
			}
			int obj_size;
			care_read(obj_size);
			std::vector<char> obj(obj_size,0);
			assert(obj.size() == obj_size);
			char* bytes = obj.data();
			care_read_s(obj_size,bytes);
			ret->emplace_back(name,clock,obj);
		}
		{
			lock l{m};
			cache[nonce] = *ret;
		}
		return ret;
	}
	catch (...){
		return nothingness;
	}
}


void CooperativeCache::listen_on(int portno){
	//fork off this thread to listen for and respond to cooperative cache requests
	std::thread([&](){
			int sockfd;
			socklen_t clilen;
			struct sockaddr_in serv_addr, cli_addr;
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
			}
			listen(sockfd,5);
			clilen = sizeof(cli_addr);
			AtScopeEnd ase{[&](){close(sockfd);}};
			discard(ase);
			while (true) {
				int newsockfd = accept(sockfd,
									   (struct sockaddr *) &cli_addr,
									   &clilen);
				if (newsockfd < 0){
					std::cerr << "ERROR on accept" << std::endl;
				}
				//fork off a new thread to handle the request.
				std::thread([&](){ respond_to_request(newsockfd);}).detach();
			}
		}).detach();
}
