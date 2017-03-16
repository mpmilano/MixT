#include "macro_utils.hpp"

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

	struct NoOverloadFoundError : MyriaException{
		const std::string mesg;
		NoOverloadFoundError(const decltype(mesg)& m):mesg(m){}
		const char * what() const noexcept {
			return mesg.c_str();
		}
	};
}
