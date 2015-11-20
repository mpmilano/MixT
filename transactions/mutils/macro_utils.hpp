#pragma once

#define VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(__VA_ARGS__,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)

//Code for generating larger VA_NARGS: j=40; echo -n '#define VA_NARGS_IMPL('; for  ((i = 1; i < $j; i = i + 1)); do echo -n '_'"$i"', '; done; echo -n 'N, ...) N'; echo; echo -n '#define VA_NARGS(...) VA_ARGS_IMPL(__VA_ARGS__,'; for ((i = $j - 1; i > 1; i = i -1)); do echo -n $i','; done; echo  '1)'


#define on_each_prn2(s,e)
#define on_each_prn3(s,a,e) s (a) e
#define on_each_prn4(s,a,b,e) s (a) e, s (b) e
#define on_each_prn5(s,a,b,c,e) s (a) e, s (b) e, s (c) e
#define on_each_prn6(s,a,b,c,d,e) s (a) e, s (b) e, s (c) e, s (d) e


#define on_each_prn_IMPL2(count, ...) on_each_prn ## count (__VA_ARGS__)
#define on_each_prn_IMPL(count, ...) on_each_prn_IMPL2(count, __VA_ARGS__)
#define on_each_prn(...) on_each_prn_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)


#define on_each1(s)
#define on_each2(s,a) s (a)
#define on_each3(s,a,b) s (a), s (b)
#define on_each4(s,a,b,c) s (a), s (b), s (c)
#define on_each5(s,a,b,c,d) s (a), s (b), s (c), s (d)


#define on_each_IMPL2(count, ...) on_each ## count (__VA_ARGS__)
#define on_each_IMPL(count, ...) on_each_IMPL2(count, __VA_ARGS__)
#define on_each(...) on_each_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)


#define pre_post_map3(sep,x,y)
#define pre_post_map4(sep,x,y,a) x a y sep
#define pre_post_map5(sep, x,y,a,b) x a y sep x b y

#define pre_post_map_IMPL2(count, ...) pre_post_map ## count (__VA_ARGS__)
#define pre_post_map_IMPL(count, ...) pre_post_map_IMPL2(count, __VA_ARGS__)
#define pre_post_map(...) pre_post_map_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)


#define CONCAT2(suff,b) b suff
#define CONCAT3(suff,b,c) b suff, c suff
#define CONCAT4(suff,b,c,d) b suff, c suff, d suff

#define CONCAT_IMPL2(count, ...) CONCAT ## count (__VA_ARGS__)
#define CONCAT_IMPL(count, ...) CONCAT_IMPL2(count, __VA_ARGS__)
#define CONCAT(...) CONCAT_IMPL(VA_NARGS(__VA_ARGS__), __VA_ARGS__)
