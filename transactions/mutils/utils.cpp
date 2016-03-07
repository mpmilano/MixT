#include "utils.hpp"
#include <arpa/inet.h>

namespace mutils{

	std::nullptr_t choose_non_np(std::nullptr_t, std::nullptr_t){
		return nullptr;
	}

	std::nullptr_t dref_np(std::nullptr_t*){
		return nullptr;
	}

	int gensym() {
		static std::mutex m;
		std::unique_lock<std::mutex> lock{m};
		static int counter = 0;
		assert(counter < (std::numeric_limits<int>::max() - 1));
		return ++counter;
	}

	std::vector<std::string> read_dir(const std::string &name){

		std::vector<std::string> ret;
	
		DIR *dir;
		struct dirent *ent;
		if ((dir = opendir (name.c_str()))) {
			/* print all the files and directories within directory */
			while ((ent = readdir (dir))) {
				std::string maybe(ent->d_name);
				if (maybe == "." || maybe == "..") continue;
				ret.push_back(std::string(maybe));
			}
			closedir (dir);
		} else {
			/* could not open directory */
			perror ("");
			assert(false && "Could not open dir.");
		}

		return ret;
	}

	void break_here(){}

	
	bool init_rand(int seed = 0){
		static std::mutex m;
		static bool init = [&](){
			std::unique_lock<std::mutex> lock{m};
			timespec ts;
			clock_gettime(CLOCK_REALTIME,&ts);
			if (seed) srand48(seed);
			else srand48(ts.tv_nsec);
			return true;}();
		return init;
	}
	
	double better_rand(){
		discard(init_rand());
		return drand48();
	}

	long int long_rand() {
		return int_rand();
	}

	unsigned int int_rand(){
		discard(init_rand());
		return lrand48();
	}

	int decode_ip(const std::string &static_addr){
		int ret = 0;
		char* iparr = (char*) &ret;
		int a, b, c, d;
		std::stringstream s(static_addr);
		char ch; //to temporarily store the '.'
		s >> a >> ch >> b >> ch >> c >> ch >> d;
		iparr[0] = a, iparr[1] = b, iparr[2] = c, iparr[3] = d;
		assert(string_of_ip(ret) == static_addr);
		return ret;
	}

	std::string string_of_ip(unsigned int i){
		if (i == 0) return "127.0.0.1";
		else {
			in_addr a;
			char str[INET_ADDRSTRLEN];
			a.s_addr = i;
			assert(a.s_addr == i);
			inet_ntop(AF_INET,&a,str,INET_ADDRSTRLEN);
			return std::string(str);
		}
	}
}

