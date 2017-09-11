#pragma once

namespace myria {
	template<typename T, template<typename> class Hndl>
	struct RemoteList {
		T value;
		Hndl<RemoteList> next;

		RemoteList(const DECT(value)& v, const DECT(next) &n)
			:value(v),
			 next(n){}
		
		//boilerplate; must find a way to eliminate it.
		bool is_struct{ true };
		auto& field(MUTILS_STRING(value)) { return value; }
		auto& field(MUTILS_STRING(next)) { return next; }
	};
}
