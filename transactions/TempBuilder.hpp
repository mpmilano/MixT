#pragma once
#include "ConStatement.hpp"

template<unsigned long long ID, typename CS>
struct DeclarationScope : public ConStatement<Level::strong>{
	const std::string name;
	const CS cs;
	DeclarationScope(const std::string &name, const CS &cs)
		:name(name),cs(cs){}
	BitSet<HandleAbbrev> getReadSet() const {
		assert(false && "again, separation?");
	}

	bool operator()(Store &s) const{
		assert(false && "TODO: these should be replaced, right?");
		return false;
	}
};

template<unsigned long long ID, typename CS>
struct all_declarations_str<DeclarationScope<ID,CS> > {
	typedef std::tuple<> type;
};

template<unsigned long long ID,typename CS>
std::ostream & operator<<(std::ostream &os, const DeclarationScope<ID,CS> &t){
	return os << t.name ;
}

template<typename PrevBuilder, unsigned long long ID, typename CS>
struct DeclarationBuilder {
	const PrevBuilder prevBuilder;
	const DeclarationScope<ID,CS> this_decl;
	typedef typename Cons<std::integral_constant<unsigned long long, ID>,
						  typename PrevBuilder::vars>::type vars;
	//TODO: should declarations modify PC label?
	typedef typename PrevBuilder::pc pc;

	DeclarationBuilder(const PrevBuilder &pb, const DeclarationScope<ID,CS> &d)
		:prevBuilder(pb),this_decl(d){}

	template<typename T>
	auto operator/(const T &t) const{
		static_assert(is_ConStatement<T>::value,
					  "Error: non-ConStatement found in IN.");
		auto new_cs = std::tuple_cat(this_decl.cs,std::tuple<T>(t));
		DeclarationScope<ID, decltype(new_cs)> new_decl(this_decl.name,new_cs);
		DeclarationBuilder<PrevBuilder, ID, decltype(new_cs)>
			r(prevBuilder,new_decl);
		return r;
	}
};


template<unsigned long long ID>
struct VarScopeBegin {
	const std::string name;
};

template<typename PrevBuilder, unsigned long long ID>
auto append(const PrevBuilder &pb, const VarScopeBegin<ID> &vsb){
	DeclarationScope<ID,std::tuple<> > dcl(vsb.name,std::tuple<>());
	DeclarationBuilder<PrevBuilder, ID, std::tuple<> > db(pb,dcl);
	return db;
}


struct VarScopeEnd {};

const auto& end_var_scope() {
	static VarScopeEnd vse;
	return vse;
}

template<typename PrevBuilder, unsigned long long ID, typename CS>
auto append(const DeclarationBuilder<PrevBuilder, ID, CS>  &pb, const VarScopeEnd&){
	return append(pb.prevBuilder, pb.this_decl);
}

#define MutDeclaration(c) VarScopeBegin<unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>{c}
#define ImmutDeclaration(c) MutDeclaration(c)


