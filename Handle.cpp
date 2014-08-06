#pragma once
#include "Backend.hpp"

namespace backend {

	class GenericHandle{
	private: 
		virtual bool is_virtual() = 0;
	};
	
	template <typename T>
	class DataStore::TypedHandle : public GenericHandle{
	private: 
		virtual bool is_virtual() = 0;
		DataStore::HandleImpl<T> &h_i;
		DataStore::HandleImpl<T> &hi() const {return h_i;}
	public:
		TypedHandle(DataStore::HandleImpl<T> &hi):h_i(hi){}
		friend class DataStore;
		template<Client_Id>
		friend class Client;
	};


	template<Client_Id id, Level L, HandleAccess HA, typename T>
	class DataStore::Handle : public DataStore::TypedHandle<T> {
	private:
		virtual bool is_virtual() {return false;}
		Handle(HandleImpl<T> &hi):TypedHandle<T>(hi){}
	public:
		static constexpr Level level = L;
		static constexpr HandleAccess ha = HA;
		typedef T stored_type;
		friend class DataStore;
		template<Client_Id>
		friend class Client;
	};

}
