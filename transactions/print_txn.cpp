#include "mailing_list_example.hpp"
#include "mtl/typecheck_printer.hpp"
#include "mtl/split_printer.hpp"
#include <iostream>

int main() {
	using namespace examples;
	using txn_t = 
	#include "mailing_list_add_new_user.cpp.precompiled"
		;
	constexpr txn_t	txn;
	std::cout << txn << std::endl;
}
