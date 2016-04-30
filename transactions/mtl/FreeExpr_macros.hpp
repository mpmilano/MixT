#pragma once
#include "macro_utils.hpp"

#define $(a,b...) make_fieldref(a,[](const std::decay_t<run_result<decltype(a)> >& a){return a.b});
