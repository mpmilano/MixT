#pragma once
#include "ponyq.hpp"

namespace mutils{

  /*
    opt out of all the specialized ponyq behavior;
    just have a Queue that supports push_new() and pop()
    only. 
   */
  template<typename T>
  struct GeneralQueue_ns{
    struct node{
      T t;
			std::atomic<node*> next{nullptr};
#ifndef NDEBUG
      std::atomic<bool> released{true};
      size_t position_estimate{0};
#endif
      template<typename... Args>
      node(Args && ... args)
	:t(std::forward<Args>(args)...){}
    };    

    struct releaser{
      template<typename... Args>
      static auto* construct_node(Args && ... args){return new node(std::forward<Args>(args)...);}
      template<typename Ignore>
      static void release_node(Ignore&, node& d){assert(d.released); delete &d;}
      releaser(...){}
    };

    using type = PonyQ<node,releaser>;
  };

  //first argument to GeneralQ's constructor
  //is  nullptr, sorry.  Or you can leave it out *if*
  //your queue elements have no constructor.
  template<typename T>
  using GeneralQueue = typename GeneralQueue_ns<T>::type;
  
}
