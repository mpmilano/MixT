#pragma once
#include <iostream>
#include "Print.hpp"
#include "Massert.hpp"

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


template<Level l2, typename T2, typename E, unsigned long long id>
std::ostream & operator<<(std::ostream &os, const RefTemporary<id,l2,T2, E>& t){
	return os << t.name <<  "<" << t.t.store_id << "," << t.id << ": " << levelStr<l2>() << ">";
}


std::ostream & operator<<(std::ostream &os, const nope& );


template<typename T>
std::ostream & operator<<(std::ostream &os, const TemporaryMutation<T>& t){
	return os << t.name << "<" << t.store_id << "> := " << t.t;
}

template<typename T, restrict(std::is_base_of<BaseFindUsages CMA T>::value)>
std::ostream & operator<<(std::ostream &os, const T& op){
	return os << op.name << "<" << get_level<T>::value << ">";
}

auto print_util(const std::shared_ptr<const std::nullptr_t>&);

template<typename T, restrict(!std::is_same<T CMA std::nullptr_t>::value)>
const auto print_util(const std::shared_ptr<const T> &sp){
	return *sp;
}

template<unsigned long long ID,typename CS, Level l, typename temp>
std::ostream & operator<<(std::ostream &os, const DeclarationScope<ID,CS,l,temp> &t){
	debug_forbid_copy = false;
//	static_assert(!std::is_same<std::decay_t<decltype(*t.gt)>, std::nullptr_t>::value,"Attempting to print DeclarationScope which has failed to find replacement!");
//	static_assert(!std::is_same<std::decay_t<decltype(t.gt.get())>, std::nullptr_t>::value,"Attempting to print DeclarationScope which has failed to find replacement!");
	assert(t.gt && "Error: we found a replacement, but gt is still null!");
	os << "let " << print_util(t.gt) << " in {";
	os << std::endl;
	fold(t.cs,[&os](const auto &e, int) -> int
		 {os << "  " << e << std::endl; return 0; },0);
	os << "}" << std::endl;
	debug_forbid_copy = true;
	return os;
}

template<unsigned long long ID,Level l, typename temp>
std::ostream & operator<<(std::ostream &os, const Temporary<ID,l,temp> &t){
	return os << t.name << "<" << l << "> = " << t.t << " @" << get_level<Temporary<ID,l,temp> >::value;
}


template<typename Cond, typename Then, typename Els>
std::ostream & operator<<(std::ostream &os, const If<Cond,Then,Els>& i){
	os << "if (" << i.cond <<") then: " << std::endl <<
		"     " << i.then;
	if (std::tuple_size<Els>::value > 0){
		os << "else: " << "     " << i.els;
	}
	return os;
}

template<typename Cond, typename Then>
std::ostream & operator<<(std::ostream &os, const While<Cond,Then>& i){
	return os << "while (" << i.cond <<") do ("
			  << (min_level<Then>::value == max_level<Then>::value ? levelStr<min_level<Then>::value>() : "mixed")
			  << "){" << i.then << "}";
}

std::ostream & operator<<(std::ostream &os, Transaction& t);

template<Level l, typename i>
std::ostream & operator<<(std::ostream &os, const CSConstant<l,i>& c){
	return os << c.val;
}

template<typename i1, typename i2>
std::ostream & operator<<(std::ostream &os, const Sum<i1,i2>& n){
	return os << n.l << " + " << n.r;
}

template<typename i1, typename i2>
std::ostream & operator<<(std::ostream &os, const Equals<i1,i2>& n){
	return os << n.l << " == " << n.r;
}

template<typename i1, typename i2>
std::ostream & operator<<(std::ostream &os, const BinaryOr<i1,i2>& n){
	return os << n.l << " || " << n.r;
}

template<typename i1, typename i2>
std::ostream & operator<<(std::ostream &os, const BinaryAnd<i1,i2>& n){
	return os << n.l << " && " << n.r;
}


template<typename i2>
std::ostream & operator<<(std::ostream &os, const Not<i2>& n){
	return os << "!" << n.v;
}

template<typename T2>
std::ostream & operator<<(std::ostream &os, const IsValid<T2> &t){
	return os << "isValid(" << t.t << ")";
}

template<Level l, HandleAccess ha, typename T>
std::ostream & operator<<(std::ostream &os, const Handle<l,ha,T>& h){
	os << "Handle<" << levelStr<l>() << ">";
	if (h.isValid()){
		os << "(" << h.get() << ")";
	}
	return os;
}

template<typename i, typename... E>
std::ostream & operator<<(std::ostream &os, const FreeExpr<i,E...>& op){
	//let's try this for now
	i ex{};
	using T = decltype(op);
	return os << ex << " @" << get_level<std::decay_t<T> >::value;
}


template<typename i>
std::ostream & operator<<(std::ostream &os, const Print<i>& op){
	return os << "print " << op.t << std::endl;
}

template<typename i>
std::ostream & operator<<(std::ostream &os, const Massert<i>& op){
	return os << "massert(" << op.t << ")" << std::endl;
}


std::ostream & operator<<(std::ostream &os, const Print_Str& op);


std::ostream & operator<<(std::ostream &os, Level l);

template<Level l>
std::ostream & operator<<(std::ostream &os, const Noop<l>&){
	
	return os << "Noop@" << l;
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

template<StoreType st>
std::ostream & operator<<(std::ostream &os, const StoreMap<st> &store){
	return os << "(a Store)";
}

template<Level l, HandleAccess ha, typename T, typename Expr>
std::ostream & operator<<(std::ostream &os, const Assignment<l,ha,T,Expr> &store){
	return os << "not printing assignments yet";
}
}
