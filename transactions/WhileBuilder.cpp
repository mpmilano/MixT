#include "WhileBuilder.hpp"

const WhileEnd& make_while_end(){
	static constexpr WhileEnd e;
	return e;
}

