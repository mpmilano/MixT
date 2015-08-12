#pragma once
#include "ConStatement.hpp"
#include "Temporary.hpp"
#include "If.hpp"

template<unsigned long long ID, typename CS, Level l>
struct DeclarationScope : public ConStatement<l>{
	const std::string name;
	const std::shared_ptr<const GeneralTemp > gt;
	const CS cs;
	const int id = gensym();
	
	template<typename Ptr>
	DeclarationScope(const std::string &name, const Ptr &gt, const CS &cs)
		:name(name),gt(gt),cs(cs){
		static_assert(can_flow(l, max_level<CS>::value),
			"Error: declaration scope requires validity check of causal expression, while body contains strong expressions.");
	}
	
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

template<unsigned long long ID, typename CS, Level l>
auto find_usage(const DeclarationScope<ID,CS,l>&){
	//we have been shadowed!
	return nullptr;
}

template<unsigned long long ID, unsigned long long ID2, typename CS, Level l, restrict(ID != ID2)>
auto find_usage(const DeclarationScope<ID2,CS,l>& ds){
	return fold(ds.cs,
				[](const auto &e, const auto &acc){
					if (!acc){
						return find_usage<ID>(e);
					}
					else return acc;
				}
				, nullptr);
}

template<unsigned long long ID,typename CS, Level l>
std::ostream & operator<<(std::ostream &os, const DeclarationScope<ID,CS,l> &t){
	assert(t.gt);
	os << t.name << "<" << levelStr<l>() <<"> = " << t.gt->gets;
	fold(t.cs,[&os](const auto &e, int) -> int
		 {os << "  " << e << std::endl; return 0; },0);
	return os;
}

template<Level l, typename T>
struct _impl_pick_new_level;

template<Level lo, unsigned long long ID, Level l, typename T>
struct _impl_pick_new_level<lo,Temporary<ID,l,T>* > : std::integral_constant<Level,l> {};

template<Level lo, unsigned long long ID, Level l, typename T>
struct _impl_pick_new_level<lo,MutableTemporary<ID,l,T>* > : std::integral_constant<Level,l> {};

template<Level l>
struct _impl_pick_new_level<l, std::nullptr_t > : std::integral_constant<Level,l> {};

template<typename PrevBuilder, unsigned long long ID, typename CS, Level l, bool oldb,
		 template<typename, unsigned long long, typename, Level, bool> typename ImplementedBuilder>
struct DeclarationBuilder {
	const PrevBuilder prevBuilder;
	const DeclarationScope<ID,CS,l> this_decl;

	DeclarationBuilder(const PrevBuilder &pb, const DeclarationScope<ID,CS,l> &d)
		:prevBuilder(pb),this_decl(d){}

	template<typename T>
	auto operator/(const T &t) const{
		static_assert(is_ConStatement<T>::value,
					  "Error: non-ConStatement found in IN.");
		auto new_cs = std::tuple_cat(this_decl.cs,std::tuple<T>(t));

		typedef typename std::decay<decltype(find_usage<ID>(t))>::type found_type;
		constexpr Level new_level = _impl_pick_new_level<l,found_type>::value;
		DeclarationScope<ID, decltype(new_cs),new_level>
			new_decl(this_decl.name,
					 (this_decl.gt ?
					  this_decl.gt :
					  make_shared<GeneralTemp>(find_usage<ID>(t))),
					 new_cs);
		ImplementedBuilder<PrevBuilder, ID, decltype(new_cs), new_level, contains_temporary<ID,T>::value || oldb>
			r(prevBuilder,new_decl);
		return r;
	}
};

template<typename PrevBuilder, unsigned long long ID, typename CS, Level l, bool oldb>
class ImmutDeclarationBuilder;

template<typename PrevBuilder, unsigned long long ID, typename CS, Level l, bool oldb>
class ImmutDeclarationBuilder : DeclarationBuilder<PrevBuilder,ID,CS,l, oldb, ImmutDeclarationBuilder>{
public:
	//TODO: should declarations modify PC label?
	typedef typename PrevBuilder::pc pc;

	ImmutDeclarationBuilder(const PrevBuilder &pb, const DeclarationScope<ID,CS,l> &d)
		:DeclarationBuilder<PrevBuilder,ID,CS,l,oldb, ::ImmutDeclarationBuilder>(pb,d) {}
};

template<typename PrevBuilder, unsigned long long ID, typename CS, Level l, bool oldb>
struct MutDeclarationBuilder;

template<typename PrevBuilder, unsigned long long ID, typename CS, Level l, bool oldb>
struct MutDeclarationBuilder : DeclarationBuilder<PrevBuilder,ID,CS,l, oldb, MutDeclarationBuilder>{
	//TODO: should declarations modify PC label?
	typedef typename PrevBuilder::pc pc;

	MutDeclarationBuilder(const PrevBuilder &pb, const DeclarationScope<ID,CS,l> &d)
		:DeclarationBuilder<PrevBuilder,ID,CS,l,oldb,::MutDeclarationBuilder>(pb,d) {}
};


template<unsigned long long ID>
struct MutVarScopeBegin {
	const std::string name;
};

template<unsigned long long ID>
struct ImmutVarScopeBegin {
	const std::string name;
};


template<typename PrevBuilder, unsigned long long ID>
auto append(const PrevBuilder &pb, const MutVarScopeBegin<ID> &vsb){
	DeclarationScope<ID,std::tuple<>,Level::strong > dcl(vsb.name,nullptr,std::tuple<>());
	MutDeclarationBuilder<PrevBuilder, ID, std::tuple<>,Level::strong, false > db(pb,dcl);
	return db;
}

template<typename PrevBuilder, unsigned long long ID>
auto append(const PrevBuilder &pb, const ImmutVarScopeBegin<ID> &vsb){
	DeclarationScope<ID,std::tuple<>,Level::strong > dcl(vsb.name,nullptr,std::tuple<>());
	ImmutDeclarationBuilder<PrevBuilder, ID, std::tuple<>,Level::strong, false > db(pb,dcl);
	return db;
}


struct VarScopeEnd {};

const auto& end_var_scope() {
	static VarScopeEnd vse;
	return vse;
}

template<typename PrevBuilder, unsigned long long ID, typename CS, Level l, bool b>
auto append(const MutDeclarationBuilder<PrevBuilder, ID, CS, l, b>  &pb, const VarScopeEnd&){
	static_assert(b,"Error: must use all temporaries you define in a transaction.");
	return append(pb.prevBuilder, pb.this_decl);
}

template<typename PrevBuilder, unsigned long long ID, typename CS, Level l, bool b>
auto append(const ImmutDeclarationBuilder<PrevBuilder, ID, CS, l, b>  &pb, const VarScopeEnd&){
	static_assert(b,"Error: must use all temporaries you define in a transaction.");
	return append(pb.prevBuilder, pb.this_decl);
}


#define MutDeclaration(c) MutVarScopeBegin<unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>{c}
#define ImmutDeclaration(c) ImmutVarScopeBegin<unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>{c}


