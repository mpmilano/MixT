#pragma once
#include "ConStatement.hpp"
#include "Temporary.hpp"
#include "If.hpp"

template<unsigned long long ID, typename CS>
struct DeclarationScope : public ConStatement<Level::strong>{
	const std::string name;
	const std::shared_ptr<const GeneralTemp> gt;
	const CS cs;
	
	template<typename Ptr>
	DeclarationScope(const std::string &name, const Ptr &gt, const CS &cs)
		:name(name),gt(gt),cs(cs){}
	
	BitSet<HandleAbbrev> getReadSet() const {
		std::cerr << "TODO: splitting, still" << std::endl;
		return 0;
	}

	bool operator()(Store &s) const{
		//Error if still null; should have been replaced by now.
		assert(gt);
		assert(false && "TODO");
		return false;
	}
};

template<unsigned long long ID, typename CS>
auto find_usage(const DeclarationScope<ID,CS>&){
	//we have been shadowed!
	return nullptr;
}

template<unsigned long long ID, unsigned long long ID2, typename CS, restrict(ID != ID2)>
auto find_usage(const DeclarationScope<ID2,CS>& ds){
	return fold(ds.cs,
				[](const auto &e, const auto &acc){
					if (!acc){
						return find_usage<ID>(e);
					}
					else return acc;
				}
				, nullptr);
}

template<unsigned long long ID,typename CS>
std::ostream & operator<<(std::ostream &os, const DeclarationScope<ID,CS> &t){
	assert(t.gt);
	os << t.name << " = " << t.gt->name;
	fold(t.cs,[&os](const auto &e, int) -> int
		 {os << "  " << e << std::endl; return 0; },0);
	return os;
}

template<typename PrevBuilder, unsigned long long ID, typename CS, bool oldb>
struct DeclarationBuilder {
	const PrevBuilder prevBuilder;
	const DeclarationScope<ID,CS> this_decl;
	//TODO: should declarations modify PC label?
	typedef typename PrevBuilder::pc pc;

	DeclarationBuilder(const PrevBuilder &pb, const DeclarationScope<ID,CS> &d)
		:prevBuilder(pb),this_decl(d){}

	template<typename T>
	auto operator/(const T &t) const{
		static_assert(is_ConStatement<T>::value,
					  "Error: non-ConStatement found in IN.");
		auto new_cs = std::tuple_cat(this_decl.cs,std::tuple<T>(t));
		DeclarationScope<ID, decltype(new_cs)> new_decl(this_decl.name,pick_useful(find_usage<ID>(t), this_decl.gt), new_cs);
		DeclarationBuilder<PrevBuilder, ID, decltype(new_cs), contains_temporary<ID,T>::value || oldb>
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
	DeclarationScope<ID,std::tuple<> > dcl(vsb.name,nullptr,std::tuple<>());
	DeclarationBuilder<PrevBuilder, ID, std::tuple<>,false > db(pb,dcl);
	return db;
}


struct VarScopeEnd {};

const auto& end_var_scope() {
	static VarScopeEnd vse;
	return vse;
}

template<typename PrevBuilder, unsigned long long ID, typename CS, bool b>
auto append(const DeclarationBuilder<PrevBuilder, ID, CS, b>  &pb, const VarScopeEnd&){
	static_assert(b,"Error: must use all temporaries you define in a transaction.");
	return append(pb.prevBuilder, pb.this_decl);
}

#define MutDeclaration(c) VarScopeBegin<unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>{c}
#define ImmutDeclaration(c) MutDeclaration(c)


