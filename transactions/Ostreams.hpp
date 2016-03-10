#pragma once
#include <iostream>
#include "Print.hpp"
#include "Massert.hpp"

namespace std{
		template<std::size_t size, typename T>
	std::ostream & operator<<(std::ostream &os, const std::array<T,size>& so){
		os << "[";
		for (auto &e : so){
			os << e << ",";
		}
		return os << "]";
	}
	
	template<typename T>
	std::ostream & operator<<(std::ostream &os, const std::vector<T>& so){
		os << "<[";
		for (auto &e : so){
			os << e << ",";
		}
		return os << "]>";
	}
}

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

	template<class Ch, class Tr, class... Args>
	auto operator<<(std::basic_ostream<Ch, Tr>& os, std::tuple<Args...> const& t)
		-> std::basic_ostream<Ch, Tr>&
	{
		aux::print_tuple(os, t, aux::gen_seq<sizeof...(Args)>());
		return os;
	}


	template<typename T>
	std::ostream & operator<<(std::ostream &os, const std::set<T>&){
		return os <<"(this is a set)";
	}

	std::ostream & operator<<(std::ostream &os, Level l);

	auto print_util(const std::shared_ptr<const std::nullptr_t>&);
	
	namespace mtl{

		template<Level l2, typename T2, typename E, unsigned long long id>
		std::ostream & operator<<(std::ostream &os, const mtl::RefTemporary<id,l2,T2, E>& t){
			return os << t.name <<  "<" << t.t.store_id << "," << t.id << ": " << levelStr<l2>() << ">";
		}


		std::ostream & operator<<(std::ostream &os, const mtl::nope& );


		template<typename T>
		std::ostream & operator<<(std::ostream &os, const mtl::TemporaryMutation<T>& t){
			return os << t.name << "<" << t.store_id << "> := " << t.t;
		}

		template<unsigned long long ID,typename CS, Level l, typename temp>
		std::ostream & operator<<(std::ostream &os, const mtl::DeclarationScope<ID,CS,l,temp> &t){
			mtl::debug_forbid_copy = false;
			//	static_assert(!std::is_same<std::decay_t<decltype(*t.gt)>, std::nullptr_t>::value,"Attempting to print DeclarationScope which has failed to find replacement!");
			//	static_assert(!std::is_same<std::decay_t<decltype(t.gt.get())>, std::nullptr_t>::value,"Attempting to print DeclarationScope which has failed to find replacement!");
			assert(t.gt && "Error: we found a replacement, but gt is still null!");
			os << "let " << print_util(t.gt) << " in {";
			os << std::endl;
			mutils::fold(t.cs,[&os](const auto &e, int) -> int
						 {os << "  " << e << std::endl; return 0; },0);
			os << "}" << std::endl;
			mtl::debug_forbid_copy = true;
			return os;
		}

		template<unsigned long long ID,Level l, typename temp>
		std::ostream & operator<<(std::ostream &os, const mtl::Temporary<ID,l,temp> &t){
			return os << t.name << "<" << l << "> = " << t.t << " @" << mtl::get_level<mtl::Temporary<ID,l,temp> >::value;
		}


		template<typename Cond, typename Then, typename Els>
		std::ostream & operator<<(std::ostream &os, const mtl::If<Cond,Then,Els>& i){
			os << "if (" << i.cond <<") then: " << std::endl <<
				"     " << i.then;
			if (std::tuple_size<Els>::value > 0){
				os << "else: " << "     " << i.els;
			}
			return os;
		}

		template<typename Cond, typename Then>
		std::ostream & operator<<(std::ostream &os, const mtl::While<Cond,Then>& i){
			return os << "while (" << i.cond <<") do ("
					  << (mtl::min_level<Then>::value == mtl::max_level<Then>::value ? levelStr<mtl::min_level<Then>::value>() : "mixed")
					  << "){" << i.then << "}";
		}

		template<typename T>
		std::ostream & operator<<(std::ostream &os, const mtl::Transaction<T>& t){
			return t.print(os);
		}

		template<typename i2>
		std::ostream & operator<<(std::ostream &os, const EnvironmentExpression<i2>&){
			return os << "evironment_expression<" << mutils::type_name<i2>() << ">";
		}

		template<Level l, typename i>
		std::ostream & operator<<(std::ostream &os, const mtl::CSConstant<l,i>& c){
			return os << c.val;
		}

		template<typename i1, typename i2>
		std::ostream & operator<<(std::ostream &os, const mtl::Sum<i1,i2>& n){
			return os << n.l << " + " << n.r;
		}

		template<typename i1, typename i2>
		std::ostream & operator<<(std::ostream &os, const mtl::Equals<i1,i2>& n){
			return os << n.l << " == " << n.r;
		}

		template<typename i1, typename i2>
		std::ostream & operator<<(std::ostream &os, const mtl::BinaryOr<i1,i2>& n){
			return os << n.l << " || " << n.r;
		}

		template<typename i1, typename i2>
		std::ostream & operator<<(std::ostream &os, const mtl::BinaryAnd<i1,i2>& n){
			return os << n.l << " && " << n.r;
		}


		template<typename i2>
		std::ostream & operator<<(std::ostream &os, const mtl::Not<i2>& n){
			return os << "!" << n.v;
		}

		template<typename T2>
		std::ostream & operator<<(std::ostream &os, const mtl::IsValid<T2> &t){
			return os << "isValid(" << t.t << ")";
		}

		template<typename i, typename... E>
		std::ostream & operator<<(std::ostream &os, const mtl::FreeExpr<i,E...>& op){
			//let's try this for now
			i ex{};
			using T = decltype(op);
			return os << ex << " @" << mtl::get_level<std::decay_t<T> >::value;
		}


		template<typename i>
		std::ostream & operator<<(std::ostream &os, const mtl::Print<i>& op){
			return os << "print " << op.t << std::endl;
		}

		template<typename i>
		std::ostream & operator<<(std::ostream &os, const mtl::Massert<i>& op){
			return os << "massert(" << op.t << ")" << std::endl;
		}


		std::ostream & operator<<(std::ostream &os, const mtl::Print_Str& op);


		template<Level l>
		std::ostream & operator<<(std::ostream &os, const mtl::Noop<l>&){
	
			return os << "Noop@" << l;
		}

		template<typename T>
		std::ostream & operator<<(std::ostream &os, const mtl::Ignore<T>& i){
	
			return os << "Ignoring " << i.t;
		}

		
		template<typename T, restrict(std::is_base_of<mtl::BaseFindUsages CMA T>::value)>
		std::ostream & operator<<(std::ostream &os, const T& op){
			return os << op.name << "<" << mtl::get_level<T>::value << ">";
		}

		template<mtl::StoreType st>
		std::ostream & operator<<(std::ostream &os, const mtl::StoreMap<st> &store){
			return os << "(a Store)";
		}

		template<typename T, typename Expr, typename Temp>
		std::ostream & operator<<(std::ostream &os, const mtl::Assignment<T,Expr,Temp> &store){
			return os << "not printing assignments yet";
		}
	}

	template<typename T, restrict(!std::is_same<T CMA std::nullptr_t>::value)>
	const auto print_util(const std::shared_ptr<const T> &sp){
		return *sp;
	}

	template<Level l, HandleAccess ha, typename T>
	std::ostream & operator<<(std::ostream &os, const Handle<l,ha,T>& h){
		os << "Handle<" << levelStr<l>() << ">";
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

