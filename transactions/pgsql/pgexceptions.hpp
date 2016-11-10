#pragma once

#include "LocalSQLConnection.hpp"

namespace myria { namespace pgsql {
		namespace local{
			struct SerializationFailure : public std::exception{
				const std::string why;
				std::shared_ptr<bool> aborting;
				SerializationFailure(LocalSQLConnection_super& conn,
									 const std::string& command_str,
									 const std::string& err_str);
				
				~SerializationFailure();
				
				const char* what() const noexcept;
			};

			struct SQLFailure : public std::exception {
				const std::string why;
				std::shared_ptr<bool> aborting;
				SQLFailure(LocalSQLConnection_super& conn,
						   const std::string& command_str,
						   const std::string& err_str);
				
				~SQLFailure();
				
				const char* what() const noexcept;
			};
		}}}
