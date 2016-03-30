#include "sync_defs.h"

int main(){

	try { 

		auto conn_causal = init_causal();
		auto conn_strong = init_strong();

		std::cout << "beginning loop" << std::endl;
		while (true){
			std::array<int,4> tmp;
			select_causal_timestamp(*conn_causal,tmp);
			update_strong_clock(*conn_strong,tmp);
			verify_strong_clock(*conn_strong,tmp);
		}
	}
	catch(const pqxx::pqxx_exception &r){
		std::cerr << r.base().what() << std::endl;
		assert(false && "exec failed");
	}
}
