#pragma once
#include "simple_rpc.hpp"
#include "DataStore.hpp"
#include "readerwriterqueue.h"
#include "transaction_listener.hpp"

namespace myria {
namespace server {

template <typename Store, typename... phases> struct StoreRelay {
  static_assert(
      std::is_base_of<DataStore<typename Store::label>, Store>::value);

  struct captive_store {
    virtual Store &store() = 0;
    virtual mutils::DeserializationManager &dsm() = 0;
    virtual ~captive_store() = default;
  };

  std::function<std::unique_ptr<captive_store>()> new_connection;

  struct StoreSession : public mutils::rpc::ReceiverFun {
    std::unique_ptr<captive_store> cstore;
    Store &store{cstore->store()};
    mutils::DeserializationManager &dsm{cstore->dsm()};
    tracker::Tracker trk;
    mutils::connection &c;
    moodycamel::BlockingReaderWriterQueue<std::vector<char>> queue;
    StoreSession(std::unique_ptr<captive_store> cstore, DECT(c) & c)
        : cstore(std::move(cstore)),
          c(c) /*,actions{[this]{this->thread_loop();}}*/ {}

    // events within thread

    void process_everything(std::size_t size, void const *const _data) {
      try {
        auto selected_txn = ((txnID_t *)_data)[0];
				#ifndef NDEBUG
				if (selected_txn == 0){
				  std::string description = typename Store::label::description{}.string;
					c.send(mutils::bytes_size(description), description);
					return;
				}
				#endif
        char const *const data = ((char *)_data) + sizeof(txnID_t);
        bool found_match =
            (false || ... || phases::run_if_match(size - sizeof(txnID_t),
                                                  selected_txn, store, trk, dsm, c, data));
				
        if (!found_match){
					std::string failure_str = std::string{"Error: no match found.  Selected txn id was "}
					+ std::to_string(selected_txn);
					c.send(false,mutils::bytes_size(failure_str),failure_str);
					assert(false);
					struct fatal{}; throw fatal{};
				}

      } catch (std::exception &e) {
        std::cout << "ERRORR: exiting with exception " << e.what();
        std::rethrow_exception(std::current_exception());
      }
    }

    void thread_loop() {
      while (!done) {
        std::vector<char> msg;
        if (queue.wait_dequeue_timed(msg, 10000)) {
          process_everything(msg.size(), msg.data());
        }
      }
    }
    std::atomic_bool done{false};
    // std::thread actions;

    // events outside of thread

    void deliver_new_event(std::size_t size, void const *const v) {
      // whendebug(bool b = ) queue.enqueue(std::vector<char>{(const char*) v,
      // ((const char*) v) + size});
      // assert(b);
      process_everything(size, v);
    }
    void async_tick() {}
    int underlying_fd() {
      return mutils::AsyncObject::always_block_code::value;
    }

    ~StoreSession() {
      done = true;
      // actions.join();
    }
  };

  std::mutex session_start_lock;
  std::unique_ptr<mutils::rpc::ReceiverFun>
  start_session(whendebug(std::ostream &, ) mutils::connection &c) {
    std::unique_lock<std::mutex> l{session_start_lock};
    return std::unique_ptr<mutils::rpc::ReceiverFun>{
        new StoreSession(new_connection(), c)};
  }
  mutils::rpc::new_connection_t start_session_wrapper =
      std::bind(&StoreRelay::start_session, this,
                std::placeholders::_1 whendebug(, std::placeholders::_2));

  mutils::simple_rpc::receiver receiver;
  StoreRelay(int port, DECT(new_connection) new_connection)
      : new_connection(std::move(new_connection)),
        receiver(port, start_session_wrapper) {
#ifndef NDEBUG
		std::cout << "Registered transactions (nontrack) " << std::endl;
		(std::cout << typename phases::normal_phase{}, ...);
		std::cout << "Registered transactions (track) " << std::endl;
		(std::cout << typename phases::tracked_phase{}, ...);
#endif
	}
};

template <typename Store, typename... transactions>
using RelayForTransactions =  StoreRelay<Store, transaction_listener<mtl::runnable_transaction::tombstone_only_phase<typename Store::label>, mtl::runnable_transaction::tombstone_only_store<typename Store::label>,mtl::runnable_transaction::tombstone_only_phase<typename Store::label>, mtl::runnable_transaction::tombstone_only_store<typename Store::label> >,
																				 listener_for<transactions,
																											typename transactions::template find_phase<
																												typename Store::label>>...>;
}
}
