#pragma once
#include "run_result.hpp"
#include "configuration_params.hpp"
#include <iostream>
#include <fstream>

namespace myria{

	std::pair<configuration_parameters, std::vector<run_result> > read_from_file(const typename run_result::time_t &now,
																				 char const * const fname){
		std::cout << fname << std::endl;
		std::pair<configuration_parameters, std::vector<run_result> > retp;
		std::vector<run_result> &ret = retp.second;
		std::ifstream file{fname};
		try {
			std::string config_line;
			std::getline(file,config_line);
			std::istringstream ignore_in{config_line};
			configuration_parameters &ignore = retp.first;
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
				}
			}
			return retp;
		}
		catch(const std::exception &e){
			std::cout << e.what() << std::endl;
			throw e;
		}
	}

	std::pair<configuration_parameters, std::vector<run_result> > read_from_files(const typename run_result::time_t &now,
																																								std::size_t count, char ** fnames){
		std::pair<configuration_parameters, std::vector<run_result> > result;
		for (std::size_t i = 0; i < count; ++i){
			auto partial = read_from_file(now,fnames[i]);
			if (i > 0) assert (same_run(result.first,partial.first));
			result.first = partial.first;
			result.second.insert(result.second.end(),partial.second.begin(),partial.second.end());
		}
		return result;
	}

}
