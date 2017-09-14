#pragma once
#include <sstream>

namespace myria {
	template<typename T, template<typename> class _Hndl>
	struct RemoteList {
		using Hndl = _Hndl<RemoteList>;
		
		T value;
		Hndl next;

		

		RemoteList(const DECT(value)& v, const DECT(next) &n)
			:value(v),
			 next(n){}
		
		//boilerplate; must find a way to eliminate it.
		bool is_struct{ true };
		auto& field(MUTILS_STRING(value)) { return value; }
		auto& field(MUTILS_STRING(next)) { return next; }
	};
}

namespace mutils {
	template<typename T, template<typename> class Hndl> struct typename_str<myria::RemoteList<T,Hndl> > {
		static std::string f(){
			std::stringstream ss;
			ss << "RemoteList<" << typename_str<T>::f() << ">";
			return ss.str();
		}
	};
}
