#pragma once

namespace myria {
	template<typename l>
	struct PreEndorse;
	
	template<typename l>
	struct PreEndorse<Label<l> >{
		using This = PreEndorse;
		//this is a label
		
	};
	template<typename l>
	struct Label<PreEndorse<l> >{
		using This = Label<typename PreEndorse<l>::This>;
		constexpr Label() = default;
		using is_label = std::true_type;
		
		template <typename T>
		constexpr static bool flows_to(const Label<PreEndorse<T> >&)
			{
				return l::flows_to(T{});
			}

		constexpr static bool flows_to(const Label<top>){
			return false;
		}
		template<char... c>
		constexpr static bool flows_to(const Label<mutils::String<c...> >,
									   std::enable_if_t<mutils::String<c...>{} != top{}>* = nullptr){
			//all pre-endorse flow to post-endorses, unless that post-endorse is top.
			return true;
		}
		//there are no writes in a pre-endorse phase, so no tracking needed here.
		using might_track = std::false_type;
		template <typename>
		using tracks_against = std::false_type;
		using run_remotely = std::false_type;
		using can_abort = std::false_type;
	};


	template<typename l>
	std::ostream &operator<<(std::ostream &o, const Label<PreEndorse<l> > &){
		return o << "Pre(" << l{} << ")";
	}

	template<typename l>
	constexpr bool is_pre_endorsed(Label<l>){
		return false;
	}
	template<typename l>
	constexpr bool is_pre_endorsed(Label<PreEndorse<l> >){
		return true;
	}

	template<typename T>
	struct PreEndorse_notop_str;

	template<>
	struct PreEndorse_notop_str<Label<top> >
	{
		using type = top;
	};
	
	template<typename l>
	struct PreEndorse_notop_str<Label<PreEndorse<l> > >
	{
		using type = PreEndorse<l>;
	};

	template<typename l>
	struct PreEndorse_notop_str{
		using type = PreEndorse<l>;
	};

	template<typename l>
	using PreEndorse_notop = typename PreEndorse_notop_str<l>::type;
}
