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

  /*
		Also support push(), via a few more variables.
		Release_node will now be responsible for ensuring
		the node goes back on the queue, and we assume that's what the user wanted.
   */

  template<typename T>
  struct ReturningQueue_ns{

		enum class location_t : char {queue, outside};

		enum class return_ready_t : char {ready, inuse};
		
    struct node{
      T t;
			std::atomic<location_t> location{location_t::queue};
			std::atomic<return_ready_t> return_ready{return_ready_t::inuse};
			std::atomic<node*> next{nullptr};
#ifndef NDEBUG
      std::atomic<bool> released{true};
      size_t position_estimate{0};
#endif
      template<typename... Args>
      node(Args && ... args)
	:t(std::forward<Args>(args)...){}

			template<typename PonyQ>
			void return_me(PonyQ& pq){
				std::atomic_store_explicit(&return_ready,return_ready_t::ready, std::memory_order_release);
				if (std::atomic_load_explicit(&location,std::memory_order_acquire) == location_t::outside) {
					//we aren't the sentinel, and we're not in the queue
					auto outside = location_t::outside;
					if (std::atomic_compare_exchange_strong(&location,&outside,location_t::queue)){
						//it's our job to do the actual enqueue.
						return_ready = return_ready_t::inuse;
						pq.push(*this);
					}
				}
			}
			
    };    

    struct releaser{
      template<typename... Args>
      static auto* construct_node(Args && ... args){return new node(std::forward<Args>(args)...);}
      template<typename PonyQ>
      static void release_node(PonyQ& q, node& n){
				assert(n.released);
				std::atomic_store_explicit(&n.location,location_t::outside, std::memory_order_release);
				if (n.return_ready == return_ready_t::ready){
					if (std::atomic_load_explicit(&n.location,std::memory_order_acquire) == location_t::outside){
						auto outside = location_t::outside;
						if(std::atomic_compare_exchange_strong(&n.location,&outside,location_t::queue)){
							n.return_ready = return_ready_t::inuse;
							q.push(n);
						}
					}
				}
			}
      releaser(...){}
    };

    using type = PonyQ<node,releaser>;
  };

  //first argument to GeneralQ's constructor
  //is  nullptr, sorry.  Or you can leave it out *if*
  //your queue elements have no constructor.
  template<typename T>
  using ReturningQueue = typename ReturningQueue_ns<T>::type;
  
	
}
