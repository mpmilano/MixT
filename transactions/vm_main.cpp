#include <iostream>
#include <pqxx/pqxx>
#include "SQLStore.hpp"
#include "FinalHeader.hpp"
#include "FinalizedOps.hpp"
#include "Ostreams.hpp"
#include "tuple_extras.hpp"
#include "Basics.hpp"
#include "backtrace.hpp"
#include <unistd.h>//*/
#include "Operate_macros.hpp"

constexpr int my_unique_id = IP_QUAD;

//const std::vector<int> personal_names_strong = {5 + (my_unique_id*100),7 + (my_unique_id*100),9 + (my_unique_id*100),11 + (my_unique_id*100),13 + (my_unique_id*100)};
const auto personal_names_causal = std::make_tuple(6 + (my_unique_id*100),8 + (my_unique_id*100),10 + (my_unique_id*100),12 + (my_unique_id*100),14 + (my_unique_id*100));

const auto lightly_names_strong = std::make_tuple(5  + (CAUSAL_GROUP * 100),7  + (CAUSAL_GROUP * 100),9  + (CAUSAL_GROUP * 100),11  + (CAUSAL_GROUP * 100),13  + (CAUSAL_GROUP * 100));
const auto lightly_names_causal = std::make_tuple(6  + (CAUSAL_GROUP * 100),8  + (CAUSAL_GROUP * 100),10  + (CAUSAL_GROUP * 100),12  + (CAUSAL_GROUP * 100),14  + (CAUSAL_GROUP * 100));

const auto heavily_names_strong = std::make_tuple(5,7/*,9,11,13*/);
const auto heavily_names_causal = std::make_tuple(6,8,10,12,14);

void setup(SQLStore<Level::strong> &strong, SQLStore<Level::causal> &causal){
	auto strongt = std::tuple_cat(lightly_names_strong, heavily_names_strong);
	auto causalt = std::tuple_cat(personal_names_causal, lightly_names_causal, heavily_names_causal);
	foreach(strongt,[&](const auto &name){
			if (!strong.exists(name)){
				strong.template newObject<HandleAccess::all>(name,1337);
			}
		});
	foreach(causalt, [&](const auto &name){
			if (!causal.exists(name)){
				causal.template newObject<HandleAccess::all>(name,1337);
			}
		});
}

int main(){

	auto writemix = [](int &generator_pos, int num_iterations, int write_percent, const auto& handles){
		struct Escape{};
		auto &i = generator_pos;
		const int modulus = 100 / write_percent;
		int j = 0;
		try{
			for (; ; ){
				foreach(handles,[&](const auto &hndl){
						std::cout << "loop " << j << " of " << num_iterations << std::endl;
						sleep(rand() %4);
						std::cout << "go" << std::endl;
						if (j >= num_iterations) throw Escape{};
						if ((i % modulus) == 0)
							TRANSACTION(
								do_op(Increment,hndl)
								)
							else hndl.get();
						++i,++j;
					});
				if (i == 100) i = 0;
			}
		}catch(const Escape&){
			
		}
	};
	
	std::cout << "hello world from VM "<< my_unique_id << " in group " << CAUSAL_GROUP << std::endl;
	int ip = 0;
	{
		char *iparr = (char*)&ip;
		//128.84.217.31
		iparr[0] = 128;
		iparr[1] = 84;
		iparr[2] = 217;
		iparr[3] = 31;
	}
	try{
		SQLStore<Level::strong> &strong = SQLStore<Level::strong>::inst(ip);
		SQLStore<Level::causal> &causal = SQLStore<Level::causal>::inst(0);
		setup(strong,causal);
		auto existing_causal = [&](const auto &name, const auto &acc){
			return tuple_cons(causal.template existingObject<HandleAccess::all, int>(name), acc);
		};
		auto existing_strong = [&](const auto &name, const auto &acc){
			return tuple_cons(strong.template existingObject<HandleAccess::all, int>(name), acc);
		};
		auto personal = fold(personal_names_causal,existing_causal,std::tuple<>());
		auto light = fold(lightly_names_strong,existing_strong,
							   fold(lightly_names_causal,existing_causal,std::tuple<>()));
		auto heavy = fold(heavily_names_strong,existing_strong,
							   fold(heavily_names_causal,existing_causal,std::tuple<>()));
		int personal_generator = 0;
		int light_generator = 0;
		int heavy_generator = 0;
		for (int i = 0; i < 5; ++i){
			writemix(light_generator,4,10,light);
			writemix(heavy_generator,1,5,heavy);
			writemix(personal_generator,5,10,personal);
		}
		
	}
	catch (const pqxx::pqxx_exception &r){
		std::cerr << r.base().what() << std::endl;
		assert(false && "exec failed");
	}
	catch(...){
		show_backtrace();
	}
}
