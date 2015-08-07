#pragma once

#define VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(__VA_ARGS__,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)

//Code for generating larger VA_NARGS: j=40; echo -n '#define VA_NARGS_IMPL('; for  ((i = 1; i < $j; i = i + 1)); do echo -n '_'"$i"', '; done; echo -n 'N, ...) N'; echo; echo -n '#define VA_NARGS(...) VA_ARGS_IMPL(__VA_ARGS__,'; for ((i = $j - 1; i > 1; i = i -1)); do echo -n $i','; done; echo  '1)'
