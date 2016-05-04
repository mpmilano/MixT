#pragma once
#include "macro_utils.hpp"

#define $(a,b...) (make_fieldref<std::decay_t<decltype(a)> CMA decltype(std::declval<run_result<std::decay_t<decltype(a)> > >().b)> (a,[](const run_result<std::decay_t<decltype(a)> >& a){return a.b;}))

#define $bld(Name,args...) (make_mtl_ctr<Name>(args...))
