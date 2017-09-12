#pragma once
#include "mutils/macro_utils.hpp"

namespace mutils{
	struct MyriaException : public std::exception {
		virtual const char* what() const noexcept = 0;
	};
	template<char... wht>
	struct StaticMyriaException : public MyriaException {
		const char* what() const noexcept {
			static const char ret[] = {wht...};
			return ret;
		}
	};
	struct FatalMyriaError : public MyriaException{
		std::string why;
		FatalMyriaError(std::string why)
			:why(why){}
		const char* what() const noexcept{
			return why.c_str();
		}
	};

	struct NoOverloadFoundError : MyriaException{
		const std::string mesg;
		NoOverloadFoundError(const decltype(mesg)& m):mesg(m){}
		const char * what() const noexcept {
			return mesg.c_str();
		}
	};
}
namespace myria{
	struct SerializationFailure : mutils::MyriaException {
		const std::string why;
		SerializationFailure(const std::string why)
			:why(why){}
		virtual const char* what() const noexcept {
			return why.c_str();
		}
	};
}
