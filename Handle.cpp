#pragma once
#include "Backend.hpp"
#include "transactions/Operation.hpp"

namespace backend {

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

		template<typename Operate, typename... OtherArgs>
		auto o(const BitSet<HandleAbbrev> &rs, const OtherArgs & ... oa){
			typedef Operation<Operate::level,Operate> Op;
			auto tuples = Op::make_tuples(*this, oa..., rs);
			return Op::operate(std::get<0>(tuples), std::get<1>(tuples), std::get<2>(tuples));
		}

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



