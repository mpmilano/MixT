namespace mutils{
	struct MyriaException : public std::exception {
		virtual const char* what() const noexcept = 0;
	};

#define MACRO_GET_1(str, i) \
	(sizeof(str) > (i) ? str[(i)] : 0)

#define MACRO_GET_4(str, i) \
	MACRO_GET_1(str, i+0),  \
		MACRO_GET_1(str, i+1),  \
		MACRO_GET_1(str, i+2),  \
		MACRO_GET_1(str, i+3)

#define MACRO_GET_16(str, i) \
	MACRO_GET_4(str, i+0),   \
		MACRO_GET_4(str, i+4),   \
		MACRO_GET_4(str, i+8),   \
		MACRO_GET_4(str, i+12)

#define MACRO_GET_64(str, i) \
	MACRO_GET_16(str, i+0),  \
		MACRO_GET_16(str, i+16), \
		MACRO_GET_16(str, i+32), \
		MACRO_GET_16(str, i+48)

#define MACRO_GET_STR(str) MACRO_GET_64(str, 0), 0 //guard for longer strings

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
