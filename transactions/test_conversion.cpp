#include "mtl/new-parsing/to-old-ast-parse.hpp"
#include "mtl/transaction_macros.hpp"
#include "mtl/transaction.hpp"
#include "mtl/parse_printer.hpp"
#include "mtl/transaction.hpp"
#include "mtl/transaction_macros.hpp"
#include "mtl/split_printer.hpp"
#include "mtl/typecheck_printer.hpp"
#include "testing_store/TestingStore.hpp"
#include <iostream>
using namespace myria;
using namespace mtl;
using namespace tracker;
using namespace mutils;

using test_struct = STRUCT(int, a, int, b, int, c);
namespace mutils{
		template<> struct typename_str<test_struct> {
			static std::string f(){return "test_struct";}
	};
}

using namespace myria;
using namespace mtl;

#define WRAP(x...)struct wrapper {\
        constexpr wrapper (){}\
        const char str[::mutils::cstring::str_len(#x)+1]{#x};\
    };\

namespace myria {
    template<> struct Label<mutils::String<'o','t','h','e','r','w','o','r','d','s'>>{};
    template<> struct Label<mutils::String<'w','o','r','d','s'>>{};
}

int main(){

  test_struct tstruct2;
  test_struct tstruct;
  tstruct.a = 0;
  tstruct.b = 0;
  tstruct.c = 0;
  tstruct2.a = 0;
  tstruct2.b = 0;
  tstruct2.c = 0;
  using Store = testing_store::TestingStore<Label<bottom> >;
  Store store;
  auto hndl = store.template newObject<int>(nullptr, 12,12);
  auto hndl2 = store.template newObject<test_struct>(nullptr,5,test_struct{});

    WRAP(var iterator = users, interator.advance(), var b1 = 89.endorse(words).otherOp(more,words), var b2 = 4894.ensure(otherwords), b2 = 2341, var b3 = *b2.field.more.field->field.isValid(), if (true || false) {return b1} else {return b2}, while (iterator.isValid()) {iterator = iterator->next}, return b1)
    parse_phase::print_ast(std::cout,convert_to_old_parsed(new_parsed<wrapper>::parse_t{}));
    constexpr auto big_one = TRANSACTION(var x = 3,
			  var y = 5,
			  x = 7,
			  y = y + x,
			  var condition = 0,
			  while (condition < x) {
			    y = y + 3,
			    condition = tstruct2.a + 1,
			    tstruct2.a = condition,
			    remote z = hndl,
			    z = condition,
			    var unused = *hndl,
			    unused = hndl2->a - 1,
			    z = unused,
			    var a = z,
			    z = a
			  },
				var ret = 0,
			  if (45 > y) {
			    y = tstruct.a,
			    tstruct.b = x
			  } else {if (hndl.isValid()){ret = 7} else {}},
				return ret
				   ).WITH(tstruct,tstruct2,hndl,hndl2);
		std::cout << typename DECT(big_one)::transaction{} << std::endl;
		
}
