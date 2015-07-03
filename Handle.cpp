#pragma once
#include "Backend.hpp"
#include "BitSet.hpp"

namespace backend {

	class HandleAbbrev{
	public:

		static constexpr std::true_type* CompatibleWithBitset = nullptr;
		const BitSet<HandleAbbrev>::member_t value;
		typedef decltype(value) itype;
		
		//dear programmer; it's on you to make sure that this is true.
		static constexpr int numbits = sizeof(decltype(value));
		template<Client_Id id, Level L, HandleAccess HA, typename T>
		friend class DataStore::Handle;

		operator decltype(value)() const {
			return value;
		}
		HandleAbbrev(decltype(value) v):value(v){}
		
		
		bool operator<(const HandleAbbrev& o) const {
			return value < o.value;
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

		//TODO: this whole HandleAbbrev thing.
		operator HandleAbbrev() const {
			assert(hi().rid < HandleAbbrev::numbits);
			HandleAbbrev::itype i = 1;
			return i << hi().rid;
		}

		HandleAbbrev abbrev() const {
			return *this;
		}
	};
}



