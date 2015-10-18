#pragma once
#include "ConStatement.hpp"

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

	DeclarationScope(const DeclarationScope& ds):
		name(ds.name),gt(ds.gt),cs(ds.cs){}

	auto handles() const {
		assert(gt);
		return std::tuple_cat(
			gt_handles(gt.get()),
			stmt_handles(cs));
	}

	bool strongCall(StrongCache& c, StrongStore &s) const {
		assert(gt);
		return strongc_helper(c,s,gt) && call_all_strong(c,s,cs);
	}
	
	bool causalCall(CausalCache& c, CausalStore &s) const {
		assert(gt);
		return causalc_helper(c,s,gt) && call_all_causal(c,s,cs);
	}

	virtual bool isVirtual() const = 0;

	
};
