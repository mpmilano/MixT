#include "Ostreams.hpp"

std::ostream & operator<<(std::ostream &os, const nope& ){
	return os << "nope!";
}

auto print_util(const std::shared_ptr<const std::nullptr_t>&){
	return "aaaaaa";
}

std::ostream & operator<<(std::ostream &os, Transaction& t){
	//see Transaction.hpp
	return t.print(os);
}

std::ostream & operator<<(std::ostream &os, const Print_Str& op){
	return os << "print " << op.t << std::endl;
}

std::ostream & operator<<(std::ostream &os, Level l){
	if (l == Level::causal)
		return os << levelStr<Level::causal>();
	else if (l == Level::strong)
		return os << levelStr<Level::strong>();
	else if (l == Level::undef)
		return os << levelStr<Level::undef>();
	assert(false && "fell through");
	return os;
}
