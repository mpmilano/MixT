#include "pgexceptions.hpp"

namespace myria { namespace pgsql {
		namespace local{

			SerializationFailure::SerializationFailure(LocalSQLConnection_super& conn,
													   const std::string& command_str,
													   const std::string& err_str)
				:why(std::string("When executing command: ")
					 + command_str
					 + std::string(" we encountered the serialization-related error ")
					 + err_str),
				 aborting(conn.aborting){
				*aborting = true;
			}

			SerializationFailure::~SerializationFailure(){
				*aborting = false;
			}

			const char* SerializationFailure::what() const noexcept {
				return why.c_str();
			}

			SQLFailure::SQLFailure(LocalSQLConnection_super& conn,
					   const std::string& command_str,
					   const std::string& err_str)
				:why(std::string("When executing command: ")
					 + command_str
						 + std::string(" we encountered the error ") + err_str),
				 aborting(conn.aborting){
				assert(!*aborting);
				*aborting = true;
			}

			SQLFailure::~SQLFailure(){
				*aborting = false;
			}
			
			const char* SQLFailure::what() const noexcept {
				return why.c_str();
			}
		}}}
