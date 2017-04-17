#pragma once
#include "run_result.hpp"
#include "configuration_params.hpp"
#include <iostream>
#include <fstream>

namespace myria{

	std::vector<run_result> read_from_file(const typename run_result::time_t &now,
																				 char const * const fname){
		std::cout << fname << std::endl;
		std::vector<run_result> ret;
		std::ifstream file{fname};
		try {
			std::string config_line;
			std::getline(file,config_line);
			std::istringstream ignore_in{config_line};
			configuration_parameters ignore;
			ignore_in >> ignore;
			std::cout << ignore << std::endl;
			{
				std::string line;
				while (std::getline(file,line)){
					assert(line.size() > 8);
					ret.emplace_back();
					std::istringstream in{line};
					char const * const c_line = line.c_str();
					(void) c_line;
					ret.back().read(now,in);
					auto cand = ret.back();
					for (int i = 1; i < ignore.log_every_n; ++i){
						ret.push_back(cand);
					}
				}
			}
			return ret;
		}
		catch(const std::exception &e){
			std::cout << e.what() << std::endl;
			throw e;
		}
	}

}
