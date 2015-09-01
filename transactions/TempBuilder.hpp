#pragma once
#include "ConStatement.hpp"
#include "Temporary.hpp"
#include "If.hpp"

bool strongc_helper(Store &, Store &, const std::shared_ptr<const std::nullptr_t>&){
	return true;
}

template<typename T, restrict(!std::is_same<T CMA std::nullptr_t>::value)>
bool strongc_helper(Store &c, Store &s, const std::shared_ptr<const T>& gt){
	return gt->strongCall(c,s);
}

bool causalc_helper(Store &, Store &, const std::shared_ptr<const std::nullptr_t>&){
	return true;
}

template<typename T, restrict(!std::is_same<T CMA std::nullptr_t>::value)>
bool causalc_helper(Store &c, Store &s, const std::shared_ptr<const T>& gt){
	return gt->causalCall(c,s);
}

template<unsigned long long ID, Level l, typename T>
auto gt_handles(Temporary<ID,l,T> const * const tmp){
	assert(tmp);
	return tmp->handles();
}

auto gt_handles(std::nullptr_t const * const){
	assert(false && "cannot retrieve handles, replacement failed!");
}

template<unsigned long long ID, typename CS, Level l, typename Temp>
struct DeclarationScope : public ConStatement<l>{
	const std::string name;
	const std::shared_ptr<const Temp > gt;
	const CS cs;
	const int id = gensym();
	
	template<typename Ptr>
	DeclarationScope(const std::string &name, const Ptr &gt, const CS &cs)
		:name(name),gt(gt),cs(cs){
	}

	auto handles() const {
		assert(gt);
		return std::tuple_cat(
			gt_handles(gt.get()),
			stmt_handles(cs));
	}

	bool strongCall(Store &c, Store &s) const {
		assert(gt);
		return strongc_helper(c,s,gt) && call_all_strong(c,s,cs);
	}
	
	bool causalCall(Store &c, Store &s) const {
		assert(gt);
		return causalc_helper(c,s,gt) && call_all_causal(c,s,cs);
	}

	virtual bool isVirtual() const = 0;

	
};

template<unsigned long long ID, unsigned long long ID2,
		 typename CS, Level l, typename Temp>
struct contains_temporary<ID,DeclarationScope<ID2, CS, l, Temp> >
	: std::integral_constant<bool,
							 ID == ID2 ||
							 contains_temporary<ID,Temp>::value ||
							 contains_temp_fold<ID,CS>::value
							 > {};

template<unsigned long long ID>
struct contains_temporary<ID,std::nullptr_t> : std::false_type {};

template<unsigned long long ID, typename T>
struct contains_temporary<ID,const T> : contains_temporary<ID,T> {};

template<unsigned long long ID, typename T>
struct contains_temporary<ID,const T&> : contains_temporary<ID,T> {};

template<unsigned long long id, Level l, typename T, typename CS>
auto isValid_desugar(Temporary<id,l,T> const * const gt, const CS &cs){
	return std::make_tuple(
		make_if(
			make_isValid(
				RefTemporary<id,l,T,Temporary<id,l,T> >{*gt})
			, cs, std::make_tuple()));
}

template<typename T, typename CS>
auto isValid_desugar(const std::shared_ptr<const T> &gt, const CS &cs){
	return isValid_desugar(gt.get(),cs);
}


template<typename CS>
auto isValid_desugar(std::nullptr_t*, const CS &cs){
	return cs;
}

template<typename CS>
auto isValid_desugar(std::nullptr_t, const CS &cs){
	return cs;
}

template<typename CS>
auto isValid_desugar(const std::shared_ptr<const std::nullptr_t>&, const CS &cs){
	return cs;
}


template<unsigned long long ID, typename CS, Level l, typename Temp>
struct ImmutDeclarationScope : public DeclarationScope<ID,CS,l,Temp>{
private:
	template<typename Ptr>
	ImmutDeclarationScope(const std::string &name, const Ptr &gt, const CS &cs)
		:DeclarationScope<ID,CS,l,Temp>(name,gt,cs) 
		{
		static_assert(can_flow(l, max_level<CS>::value),
			"Error: declaration scope requires validity check of causal expression, while body contains strong expressions.");
		}
public:
	bool isVirtual() const {return false;}

	template<typename Ptr>
	static auto build(const std::string &name, const Ptr &gt, const CS &cs){
		return ImmutDeclarationScope<ID,CS,l,Temp>{name,gt,cs};
	}
};

template<unsigned long long ID, typename CS, Level l, typename Temp, typename Ptr>
auto build_ImmutDeclarationScope(const std::string &name, const Ptr &gt, const CS &cs){
	auto new_cs = isValid_desugar(gt,cs);
	return ImmutDeclarationScope<ID,decltype(new_cs),l,Temp>::build(name,gt,new_cs);
}

template<unsigned long long ID, typename CS, Level l, typename Temp>
struct MutDeclarationScope : public DeclarationScope<ID,CS,l,Temp>{
	
	template<typename Ptr>
	MutDeclarationScope(const std::string &name, const Ptr &gt, const CS &cs)
		:DeclarationScope<ID,CS,l,Temp>(name,gt,cs){}
	bool isVirtual() const {return false;}
};

template<unsigned long long ID, unsigned long long ID2,
		 typename CS, Level l, typename Temp>
struct contains_temporary<ID,MutDeclarationScope<ID2, CS, l, Temp> >
	: contains_temporary<ID,DeclarationScope<ID2,CS,l,Temp> > {};

template<unsigned long long ID, unsigned long long ID2,
		 typename CS, Level l, typename Temp>
struct contains_temporary<ID,ImmutDeclarationScope<ID2, CS, l, Temp> > 
	: contains_temporary<ID,DeclarationScope<ID2,CS,l,Temp> > {};

template<unsigned long long ID, typename CS, Level l, typename Temp >
std::nullptr_t find_usage(const DeclarationScope<ID,CS,l,Temp>&){
	//we have been shadowed!
	return nullptr;
}

template<unsigned long long ID, unsigned long long ID2, typename CS, Level l, typename temp, restrict(ID != ID2)>
auto find_usage(const DeclarationScope<ID2,CS,l,temp>& ds){
	return fold(ds.cs,
				[](const auto &e, const auto &acc){
					return choose_non_np(acc,find_usage<ID>(e));
				}
				, nullptr);
}

struct BaseFindUsages {};

template<typename... T>
struct FindUsages : public BaseFindUsages {
	const std::tuple<T...> find_from_here;
	FindUsages(const T & ... t):find_from_here(std::make_tuple(t...)){}
};

template<unsigned long long ID, typename T,
		 restrict(std::is_base_of<BaseFindUsages CMA T>::value)>
auto find_usage(const T& fu) {
	return fold(fu.find_from_here,
				[](const auto &e, const auto &acc){
					return choose_non_np(find_usage<ID>(e),acc);
				}
				, nullptr);
}


template<typename , typename >
struct _impl_pick_new_type;

template<unsigned long long ID, Level l, typename T>
struct _impl_pick_new_type<Temporary<ID,l,T>*,Temporary<ID,l,T> > {
	using type = Temporary<ID,l,T>;
};

template<unsigned long long ID, Level l, typename T>
struct _impl_pick_new_type<MutableTemporary<ID,l,T>*,MutableTemporary<ID,l,T> > {
	using type = MutableTemporary<ID,l,T>;
};

template<typename T>
struct _impl_pick_new_type<T*, std::nullptr_t > {
	using type = T;
};

template<typename T>
struct _impl_pick_new_type<std::nullptr_t, T > {
	using type = T;
};

template<>
struct _impl_pick_new_type<std::nullptr_t, std::nullptr_t > {
	using type = std::nullptr_t;
};

//ugh truth tables.
template<unsigned long long id, Level l2, typename T>
auto choose_gt(const std::shared_ptr<const std::nullptr_t>&, const std::shared_ptr<const MutableTemporary<id,l2,T> >& r){
	assert(r.get());
	return r;
}
template<unsigned long long id, Level l2, typename T>
auto choose_gt(const std::shared_ptr<const std::nullptr_t>&, const std::shared_ptr<const Temporary<id,l2,T> >& r){
	assert(r.get());
	return r;
}
template<unsigned long long id, Level l2, typename T>
auto choose_gt(const std::shared_ptr<const MutableTemporary<id,l2,T> >& r,const std::shared_ptr<const std::nullptr_t>&){
	assert(r.get());
	return r;
}
template<unsigned long long id, Level l2, typename T>
auto choose_gt(const std::shared_ptr<const Temporary<id,l2,T> >& r,const std::shared_ptr<const std::nullptr_t>&){
	assert(r.get());
	return r;
}

template<typename T, restrict(!std::is_same<T CMA std::shared_ptr<const std::nullptr_t> >::value )>
auto choose_gt(const T& r1, const T& r2){
	assert(r1.get() || r2.get());
	if (r1.get()) return r1;
	else return r2;
}

template<typename T, restrict2(std::is_same<T CMA std::shared_ptr<const std::nullptr_t> >::value )>
auto choose_gt(const T&, const T& r){
	return r;
}


template<typename T, typename R>
auto choose_gt(const T&, const R& r){
	static_assert(std::is_same<T CMA T*>::value, "What in the nine hells?");
	return r;
}

template<typename PrevBuilder, unsigned long long ID, typename CS, Level l, bool oldb, typename Temp>
struct MutableDeclarationBuilder {
	const PrevBuilder prevBuilder;
	const MutDeclarationScope<ID,CS,l,Temp> this_decl;
	typedef typename PrevBuilder::pc pc;

	MutableDeclarationBuilder(const PrevBuilder &pb, const MutDeclarationScope<ID,CS,l,Temp> &d)
		:prevBuilder(pb),this_decl(d){}

	template<typename T>
	auto operator/(const T &t) const{
		static_assert(is_ConStatement<T>::value,
					  "Error: non-ConStatement found in IN.");
		auto new_cs = std::tuple_cat(this_decl.cs,std::tuple<T>(t));

		typedef typename _impl_pick_new_type<
			typename std::decay<decltype(dref_np(find_usage<ID>(t)))>::type,
			typename std::decay<decltype(*this_decl.gt.get())>::type
			>::type found_type;

		std::cout << "looking for " <<  ID << std::endl;
		if (contains_temporary<ID,found_type>::value){
			std::cout << "found it!" << std::endl;
		}
		else {
			std::cout << "still looking" << std::endl;
		}
		
		constexpr Level new_level = get_level<found_type>::value;
		MutDeclarationScope<ID, decltype(new_cs),new_level,found_type>
			new_decl(this_decl.name,
					 choose_gt(
						 this_decl.gt,
						 make_cnst_shared<found_type>(find_usage<ID>(t))),
					 new_cs);
		::MutableDeclarationBuilder<PrevBuilder, ID, decltype(new_cs), new_level, contains_temporary<ID,found_type>::value || oldb, found_type>
			r(prevBuilder,new_decl);
		return r;
	}
};

template<typename PrevBuilder, unsigned long long ID, typename CS, Level l, bool oldb, typename Temp>
struct ImmutableDeclarationBuilder {
	const PrevBuilder prevBuilder;
	const ImmutDeclarationScope<ID,CS,l,Temp> this_decl;
	typedef typename PrevBuilder::pc pc;

	ImmutableDeclarationBuilder(const PrevBuilder &pb, const ImmutDeclarationScope<ID,CS,l,Temp> &d)
		:prevBuilder(pb),this_decl(d){}

	template<typename T>
	auto operator/(const T &t) const{
		static_assert(is_ConStatement<T>::value,
					  "Error: non-ConStatement found in IN.");
		auto new_cs = std::tuple_cat(this_decl.cs,std::tuple<T>(t));

		typedef typename _impl_pick_new_type<
			typename std::decay<decltype(find_usage<ID>(t))>::type,
			typename std::decay<decltype(*this_decl.gt)>::type
			>::type found_type;

		std::cout << "looking for (immutable) " << ID << std::endl;
		if (contains_temporary<ID,found_type>::value){
			std::cout << "found it!" << std::endl;
		}
		else {
			std::cout << "still looking" << std::endl;
		}


		constexpr Level new_level = get_level<found_type>::value;
		auto new_decl = build_ImmutDeclarationScope<ID, decltype(new_cs),new_level,found_type>
			(this_decl.name,
					 choose_gt(
						 this_decl.gt,
						 make_cnst_shared<found_type>(find_usage<ID>(t))),
					 new_cs);
		::ImmutableDeclarationBuilder<PrevBuilder, ID, std::decay_t<decltype(new_decl.cs)>, new_level, contains_temporary<ID,found_type>::value || oldb, found_type>
			r(prevBuilder,new_decl);
		return r;
	}
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
	MutDeclarationScope<ID,std::tuple<>,Level::strong,std::nullptr_t > dcl(vsb.name,nullptr,std::tuple<>());
	MutableDeclarationBuilder<PrevBuilder, ID, std::tuple<>,Level::strong, false, std::nullptr_t > db(pb,dcl);
	return db;
}

template<typename PrevBuilder, unsigned long long ID>
auto append(const PrevBuilder &pb, const ImmutVarScopeBegin<ID> &vsb){
	auto dcl = build_ImmutDeclarationScope<ID,std::tuple<>,Level::strong,std::nullptr_t>(vsb.name,nullptr,std::tuple<>());
	ImmutableDeclarationBuilder<PrevBuilder, ID, std::tuple<>,Level::strong, false, std::nullptr_t > db(pb,dcl);
	return db;
}


struct VarScopeEnd {};

const auto& end_var_scope() {
	static VarScopeEnd vse;
	return vse;
}

template<typename A, typename B>
auto append_helper(const A& a, const B &b, std::true_type*){
	return append(a, b);
}

template<typename A, typename B>
auto append_helper(const A& prevBuilder, const B &this_decl, std::false_type*){
	return fold(
		this_decl.cs,
		[](const auto &e, const auto &acc){
			return append(acc,e);
		},
		prevBuilder
		);
}

template<typename PrevBuilder, unsigned long long ID, typename CS, Level l, bool b, typename temp>
auto append(const MutableDeclarationBuilder<PrevBuilder, ID, CS, l, b,temp>  &pb, const VarScopeEnd&){
	if (!b) std::cout << "we just ended a var scope without finding anything ("
					  << ID << ")" << std::endl;
	std::integral_constant<bool, b> *choice = nullptr;
	return append_helper(pb.prevBuilder, pb.this_decl, choice);
}

template<typename PrevBuilder, unsigned long long ID, typename CS, Level l, bool b, typename temp>
auto append(const ImmutableDeclarationBuilder<PrevBuilder, ID, CS, l, b, temp>  &pb, const VarScopeEnd&){
	if (!b) std::cout << "we just ended a var scope without finding anything ("
					  << ID << ")" << std::endl;
	std::integral_constant<bool, b> *choice = nullptr;
	return append_helper(pb.prevBuilder, pb.this_decl, choice);
}


#define MutDeclaration(c) MutVarScopeBegin<unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>{c}
#define ImmutDeclaration(c) ImmutVarScopeBegin<unique_id<(sizeof(c) / sizeof(char)) - 1>(c)>{c}


