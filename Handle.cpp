#pragma once
#include "Backend.hpp"

namespace backend {

	class HandleAbbrev{
	public:

		template<Client_Id id, Level L, HandleAccess HA, typename T>
		friend class DataStore::Handle;

		bool operator<(const HandleAbbrev&) const {
			return false;
		}
		//idea; we use this for tracking the ReadSet.
	};
	
	template<Client_Id id, Level L, HandleAccess HA, typename T>
	class DataStore::Handle {
	private:
		virtual bool is_virtual() {return false;}
		DataStore::HandleImpl<T> &h_i;
		DataStore::HandleImpl<T> &hi() const {return h_i;}
		Handle(HandleImpl<T> &hi):h_i(hi){}
	public:
		static constexpr Level level = L;
		static constexpr HandleAccess ha = HA;
		typedef T stored_type;
		friend class DataStore;
		template<Client_Id>
		friend class Client;
		const T& get() const {
			return *hi().stored_obj;
		}

		Handle clone() const {
			return *this;
		}

		void put(const T& t) {
			hi().stored_obj.reset(new T(t));
		}

		template<typename T2, typename... RS>
		auto o(RS...){
			return T2(this)();
		}

		operator HandleAbbrev(){
		}
	};
}



