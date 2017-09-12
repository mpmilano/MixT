#pragma once
#include <iostream>
#include <set>
#include "mutils-containers/TrivialPair.hpp"
#include "Handle.hpp"

namespace mutils{
		
	template<typename A, typename B, typename C>
	std::ostream & operator<<(std::ostream &os, const TrivialTriple<A,B,C>& so){
		return os << "<" << so.first << "," << so.second << "," << so.third << ">";
	}
}

namespace myria{

	namespace aux{
		template<std::size_t...> struct seq{};

		template<std::size_t N, std::size_t... Is>
		struct gen_seq : gen_seq<N-1, N-1, Is...>{};

		template<std::size_t... Is>
		struct gen_seq<0, Is...> : seq<Is...>{};

		template<class Ch, class Tr, class Tuple, std::size_t... Is>
		void print_tuple(std::basic_ostream<Ch,Tr>& os, Tuple const& t, seq<Is...>){
			using swallow = int[];
			(void)swallow{0, (void(os << (Is == 0? "" : ", ") << std::get<Is>(t)), 0)...};
		}
	} // aux::

    template<class... Args>
    std::ostream& operator<<(std::ostream& os, const std::tuple<Args...> & t)
	{
		aux::print_tuple(os, t, aux::gen_seq<sizeof...(Args)>());
		return os;
	}


	template<typename T>
	std::ostream & operator<<(std::ostream &os, const std::set<T>&){
		return os <<"(this is a set)";
	}


	auto print_util(const std::shared_ptr<const std::nullptr_t>&);	


	template<typename T>
	const auto& print_util(const std::shared_ptr<const T> &sp, std::enable_if_t<!std::is_same<T,std::nullptr_t>::value>* = nullptr){
		return *sp;
	}

	template<typename l, typename T,typename... Ops>
    std::ostream & operator<<(std::ostream &os, const Handle<l,T,Ops...>& ){
		os << "Handle<" << l{} << ">";
		return os;
	}

	template<typename T>
	std::ostream & operator<<(std::ostream &os, const std::list<std::unique_ptr<T> > &lst){
		os << "[list: ";
		for (const auto &ptr : lst){
			os << "; " << *ptr;
		}
		os << "]";
		return os;
	}

    namespace tracker {
		std::ostream& operator<<(std::ostream& os, const Tracker::Tombstone &tmb);

    }
	
}

