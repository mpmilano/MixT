#pragma once

//this makes sure the expression (expr)
//only references temporaries defined in the tuple
//Names
#define NAME_CHECK(Names,expr)

struct EmptySeq {};
constexpr EmptySeq empty_seq;
