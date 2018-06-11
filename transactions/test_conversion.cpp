#include "mtl/new-parsing/to-old-ast-parse.hpp"
#include "mtl/transaction_macros.hpp"
#include "mtl/transaction.hpp"
#include "mtl/parse_printer.hpp"

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
    WRAP(var iterator = users, interator.advance(), var b1 = 89.endorse(words).otherOp(more,words), var b2 = 4894.ensure(otherwords), b2 = 2341, var b3 = *b2.field.more.field->field.isValid(), if (true || false) {return b1} else {return b2}, while (iterator.isValid()) {iterator = iterator->next}, return b1)
    parse_phase::print_ast(std::cout,convert_to_old_parsed(new_parsed<wrapper>::parse_t{}));
}
