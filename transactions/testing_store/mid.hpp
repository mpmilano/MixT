#pragma once
#include "mtl/top.hpp"

namespace myria{
	using mid = MUTILS_STRING(mid);
	template<>
	struct Label<mid>{
		constexpr Label() = default;
		using is_label = std::true_type;

		constexpr static bool flows_to(const Label<top>){
			return false;
		}

		constexpr static bool flows_to(const Label<bottom>){
			return true;
		}

		template<typename l>
		constexpr static bool flows_to(const Label<PreEndorse<l> >){
			return false;
		}

		constexpr static bool flows_to(const Label){
			return true;
		}		

		//there are no writes in a pre-endorse phase, so no tracking needed here.
		using might_track = std::false_type;
		template <typename>
		using tracks_against = std::false_type;
		using run_remotely = std::false_type;
		using can_abort = std::false_type;
	};

	inline std::ostream &operator<<(std::ostream &o, const Label<mid> &){
		return o << "mid";
	}
}
