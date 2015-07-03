#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"
#include "Seq.hpp"
#include "../Backend.hpp"
#include <iostream>

using namespace backend;

template<typename Expr>
class Assignment : ConStatement<get_level<Expr>::value> {
	static constexpr Level level = get_level<Expr>::value;
	static_assert(is_ConExpr<Expr>::value,"must assign result of expression only!");
		
public:

	const std::function<void ()> thunk;
	std::set<backend::HandleAbbrev> rs;
	
	template<Client_Id id, Level l, HandleAccess HA, typename T>
	Assignment(DataStore::Handle<id,l,HA,T> h, Expr e)
		:thunk([=](){return h.clone().put(e);}),
		 rs(get_ReadSet(e))
		{
		static_assert(canWrite(HA),"Error: needs writeable handle!");
		static_assert(l == Level::causal || level == Level::strong,"Error: flow violation");
	}
	
	decltype(rs) getReadSet() const {
		return rs;
	}


	void operator()(){
		thunk();
	}

};

template<Level l, Client_Id id, HandleAccess HA, typename T, typename Expr>
Assignment<Expr> operator<<(DataStore::Handle<id,l,HA,T> &sink, Expr source) {
	return Assignment<Expr>(sink,source);
}
