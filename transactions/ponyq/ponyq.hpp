#pragma once
#include <cassert>
#include <atomic>
#include <thread>
#include <iostream>
#include "mutils/macro_utils.hpp"

namespace mutils{


  /*
    This queue is taken from the Pony project's message queue.
    The requirements on the template parameters are encoded in static asserts.
    _node is the queue node type; we require that it has a next pointer.
    MemoryManager is used to create + release nodes.  
   */
template<typename _node, typename MemoryManager>
struct PonyQ{

  //static_asserts to determine if parameters are correct.
  using node = _node;
  static_assert(std::is_same<std::decay_t<decltype(std::declval<node>().next)>, std::atomic<node*>>::value,
		"Error: node does not contain next field of type atomic<node*>");
  whendebug(static_assert(std::is_same<std::decay_t<decltype(std::declval<node>().released)>, std::atomic<bool>>::value,
			  "Error: node does not contain released field of type atomic<bool>"));
  
  template<typename R, typename... Args>
  constexpr static bool is_fp(R (MemoryManager::*) (Args...)){
    return true;
  }

  template<typename R, typename... Args>
  constexpr static bool is_fp(R (MemoryManager::*) (Args...) const ){
    return true;
  }

  template<typename R, typename... Args>
  constexpr static bool is_fp(R (*) (Args...)){
    return true;
  }  

  static_assert(PonyQ::is_fp<void,PonyQ&,node&>(&MemoryManager::release_node),
		"Error: Memory Manager does not contain single release_node() function of type PonyQ& -> node& -> void");

  //END STATIC ASSERTS, BEGIN CODE PROPER.

  /*
    False head tracks the (logical) head of the queue.  
    this is usually just an alias for the sentinel, 
    but can diverge from the sentinel if an element is added at the head
    (or the queue is popped to empty). 
   */
  struct false_head{
  private:
    PonyQ &parent;
    bool is_extra{false};
    node* head{nullptr};
    inline void destroy(){
      if (is_extra) {
	parent.d.release_node(parent,*head);
	whendebug(head = nullptr);
	is_extra = false;
      }
    }
  public:
    //ONLY FOR USE when aliasing existing queue element
    auto& operator=(node* n){
      destroy();
      head = n;
      return *this;
    }
    
    auto* operator->() const {
      return head;
    }
    auto* operator->() {
      return head;
    }

    //ONLY FOR USE when creating new queue element to
    //"insert" at the head.
    void set_extra(node* n){
      assert(!is_extra);
      assert(!head);
      is_extra = true;
      head = n;
    }
    ~false_head(){destroy();}
    false_head(PonyQ &parent):parent(parent){}
    operator node*() const {return head;}
    operator bool() const {return head;}
  };

  std::atomic<node*> tail;
  //logical head
  false_head head{*this};
  //literal head (cannot be null)
  node* sentinel;
  //for creating/destroying nodes
  MemoryManager d;

  /*
    First argument to PonyQ constructor is for MemoryManager.
    All subsequent arguments are for constructing the seed node.
    PonyQ is always initialized non-empty.
   */
  template<typename... Args>
  void init(Args && ... args)
  { 
    node* stub = d.construct_node(std::forward<Args>(args)...);
    constexpr node* np = nullptr;
    std::atomic_store_explicit(&stub->next, np, std::memory_order_relaxed);
    
    std::atomic_store_explicit(&tail, (node*)((uintptr_t)stub | 1),
				  std::memory_order_relaxed);
    sentinel = stub;
    head = stub;
  }
  template<typename... Args>
  PonyQ(Args && ... args){
    init(std::forward<Args>(args)...);
  }

  ~PonyQ() {
    //Need to drain the queue,
    //prevent any valid returns from hitting the tail,
    //and delete the sentinel.
    using namespace std;
    while (peek()) pop();
    constexpr node* np = nullptr;
    std::atomic_store_explicit(&this->tail, np, std::memory_order_relaxed);
    assert(this->head == nullptr);
    assert(this->sentinel != nullptr);
    auto* sentinel = (node*)((uintptr_t)this->sentinel & ~(uintptr_t)1);
    assert(!sentinel->released);
    whendebug(sentinel->released = true);
    d.release_node(*this,*sentinel);
    this->sentinel = nullptr;
    this->head = nullptr;
    }//*/

  //thread-safe, must be released messages originating
  //*from this queue*
  void push(node& msg){
    assert(msg.released);
    whendebug(msg.released = false);
    using namespace std;
    constexpr node* np = nullptr;
    node *m = &msg;
    std::atomic_store_explicit(&m->next, np, std::memory_order_relaxed);
    node* prev = std::atomic_exchange_explicit(&this->tail, m,
						  std::memory_order_relaxed);
    prev = (node*)((uintptr_t)prev & ~(uintptr_t)1);
    
    std::atomic_store_explicit(&prev->next, m, std::memory_order_release);
		//if (blocking_mode){ blocker.notify_all();}
  }

  //only thread-safe if d.construct_node is thread-safe
  template<typename... Args>
  void push_new(Args && ... args){
    auto tmp = d.construct_node(std::forward<Args>(args)...);
    assert(tmp->released == true);
    assert(tmp->next == nullptr);
    return push(*tmp);
  }

  //not thread-safe
  //also seems like can't migrate between threads.
  void pop(){
    using namespace std;
    node* sentinel = this->sentinel;
    node* next = std::atomic_load_explicit(&sentinel->next, std::memory_order_relaxed);
    head = next;
    if(next != nullptr){
      this->sentinel = next;
      std::atomic_thread_fence(std::memory_order_acquire);
      whendebug(sentinel->released = true);
      d.release_node(*this,*sentinel);
    }
  }

  //not thread-safe unless queue is known to be non-empty
  node* peek(){
    if (!head) pop();
    return head;
  }

	std::condition_variable blocker;
	std::mutex blocker_lock;
	bool blocking_mode{false};
	node& blocking_pop(){
		using namespace std::chrono;
		auto cand = peek();
		while (!cand){
			//std::unique_lock<std::mutex> blocker_lock{};
			//blocker.wait(blocker_lock,[this]{return this->peek();});
			cand = peek();
		}
		pop();
		return *cand;
	}

  std::size_t total_allocations{0};

  //return the head element, or construct a new one
  //if the head element is null.
  //not thread-safe unless queue is known to be non-empty
  template<typename... Args>
  node* peek_or_build(Args && ...args){
    if (!peek()) {
      head.set_extra(d.construct_node(std::forward<Args>(args)...));
    }
    return head;
  }
};
}
