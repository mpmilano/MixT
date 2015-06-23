#pragma once
#include "ConStatement.hpp"
#include "BaseCS.hpp"
#include "ConExpr.hpp"

class If;

#define handle_level backend::handle_level

template<typename Cond, typename Then, typename Els,
		 restrict(
			 //concepts
			 is_ConStatement<Then>::value && is_ConStatement<Els>::value
			 && is_ConExpr<Cond>::value 
			 &&
			 //enforce consistency flow direction
			 ((get_level<Cond>::value == Level::causal &&
			   get_level<Then>::value == Level::causal &&
			   get_level<Els>::value == Level::causal)
			  ||
			  (get_level<Cond>::value == Level::strong))
			 )>
auto make_if(const Cond& c, const Then &t, const Els &e){
	return 0;
}

class If {
	
};
