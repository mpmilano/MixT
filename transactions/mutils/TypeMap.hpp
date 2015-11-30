//A compile-time map from types -> tuples.

#pragma once
#include <tuple>
#include "utils.hpp"

namespace ctm {

  typedef std::tuple<> empty_map;

  struct not_found {
    typedef std::false_type found;
  };

  template<typename Key, typename V1, typename V2>
  struct entry {
    typedef Key key;
    typedef V1 v1;
    typedef V2 v2;
    typedef std::true_type found;
  };
	
  template<unsigned long long Key, typename Value1, typename Value2, typename Map>
  using insert =
    typename
    Cons< entry<
	    typename std::integral_constant<unsigned long long, Key>::type,
	    Value1,Value2>,Map>::type;


  template<typename A, typename B>
  using Fun = typename std::conditional<
    std::is_same<
      typename A::key,
      std::integral_constant<unsigned long long,
			     std::decay<decltype(
						 std::get<std::tuple_size<B>::value - 1>
						 (mke<B>()))>::type::value>
      >::value,
    typename Cons<A, B>::type,
    B
    >::type;

  template<unsigned long long Key, typename Map>
  struct _Find{
    typedef typename std::integral_constant<unsigned long long,Key>::type key;
    typedef typename std::decay<
      decltype(std::get<0>
	       (mke<fold_types<Fun, Map, std::tuple<key> > > ()))
      >::type res;
    typedef typename std::conditional<
      std::is_same<res,key>::value,
      not_found,
      res>::type type;
  };
	
  template<unsigned long long Key, typename Map>
  using find = typename _Find<Key,Map>::type;
	
	
  /*	template<int where, unsigned long long Key, typename Map>
	constexpr auto _find_loc(Map m){
	return (ct::get<where>(m) == Key ? where : _find_loc<where + 1,Key>(m));
	}
	
	template<unsigned long long Key, typename Map, Map m>
	constexpr auto find(){
	constexpr int loc = _find_loc(m);
	struct entry{
	typedef typename decltype(ct::get<loc>(m))::v1 v1;
	const decltype(ct::get<loc>(m).v2) v2;
	} e{ct::get<loc>(m).v2};
	return e;
	}
  */
}
