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
			configuration_parameters ignore;
			file >> ignore;
			std::cout << ignore << std::endl;
			while (file.good()){
				ret.emplace_back();
				ret.back().read(now,file);
				auto cand = ret.back();
				for (int i = 1; i < ignore.log_every_n; ++i){
					ret.push_back(cand);
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
