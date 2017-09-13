#pragma once
#include "configuration_params.hpp"
#include "run_result.hpp"
#include "relay_connections_pool.hpp"
#include "test_client.hpp"
#include "mutils-tasks/blockingconcurrentqueue.h"
#include "mutils-tasks/ctpl_stl.h"

namespace myria {

struct test {
  configuration_parameters params;

#ifndef NOPOOL
  pgsql::SQLConnectionPool<pgsql::Level::strong> spool;
  pgsql::SQLConnectionPool<pgsql::Level::causal> cpool;
#else
  const std::string spool{mutils::string_of_ip(params.strong_ip)};
  const std::string cpool{mutils::string_of_ip(params.causal_ip)};
#endif
  relay_connections_pool causal_connections{params, params.causal_ip,
                                            params.causal_relay_port,
                                            (int)params.max_clients()};
  relay_connections_pool strong_connections{params, params.strong_ip,
                                            params.strong_relay_port,
                                            (int)params.max_clients()};

  moodycamel::BlockingConcurrentQueue<std::unique_ptr<client>> client_queue;
  std::atomic<std::size_t> number_enqueued_clients{0};
  void push_client() {
    client_queue.enqueue(std::make_unique<client>(
        *this, spool, cpool, strong_connections.weakspawn(),
        causal_connections.weakspawn()));
    ++number_enqueued_clients;
  }
  ctpl::thread_pool tp{(int)std::max(params.max_clients() / 2, params.starting_num_clients)};

  test(configuration_parameters params) : params(params) {
    output_file << params << std::endl;
    for (std::size_t i = 0; i < params.starting_num_clients; ++i) {
      push_client();
    }
  }

  auto now() { return std::chrono::high_resolution_clock::now(); }

  template <typename Time>
  auto schedule_event(const Time &start_time, std::size_t parallel_factor) {
    return now() +
           getArrivalInterval(params.current_arrival_rate(now() - start_time) /
                              parallel_factor);
  }

  template <typename timeout, typename time>
  void process_results(moodycamel::ConcurrentQueue<
                           std::future<std::unique_ptr<run_result>>> &results,
                       timeout delay, time next_event_time, time start_time) {
    std::future<std::unique_ptr<run_result>> it;
    while (next_event_time > now() && results.try_dequeue(it)) {
      if (it.wait_for(delay) != std::future_status::timeout) {
        try {
          this->print_result(start_time, *it.get());
        } catch (...) {
          run_result r;
          r.is_fatal_error = true;
          this->print_result(start_time, r);
          --number_enqueued_clients;
        }
      } else {
        if (it.valid())
          results.enqueue(std::move(it));
      }
    }
    output_file.flush();
  }

  std::ofstream output_file{params.output_file};

  template <typename time>
  void print_result(const time &start_time, const run_result &r) {
    r.print(start_time, output_file);
  }
  void run_test() {
    using namespace server;
    using namespace pgsql;
    using namespace mtl;
    using namespace std;
    using namespace chrono;
    using namespace mutils;
    using namespace moodycamel;
    std::size_t parallel_factor = std::min<std::size_t>(params.parallel_factor,tp.size());
		assert(parallel_factor > 0);
    ConcurrentQueue<std::future<std::unique_ptr<run_result>>> results;
    auto start_time = now();
    DECT(start_time) last_log_write_time = start_time;
    try {
      std::vector<std::future<void>> launch_threads;
      for (std::size_t i = 0; i < parallel_factor; ++i) {
        launch_threads.emplace_back(std::async(launch::async, [&] {
          this->launcher_loop(results, start_time, parallel_factor);
        }));
      }
      for (auto &e : launch_threads) {
        // do some work in the meantime
        while (e.wait_for(1s) != future_status::ready) {
          // increase client pool based on elapsed time
          while (number_enqueued_clients <
                 params.total_clients_at(now() - start_time)) {
            push_client();
          }

          // if it's been forever since our last log write, write the log *now*
          if (now() > (params.log_delay_tolerance + last_log_write_time)) {
            std::cout << "log writer stalled, flushing now" << std::endl;
            process_results(results, 0s, 1s + now(), start_time);
            last_log_write_time = now();
          }
        }
        e.get();
      }
    } catch (...) {
      // Looks like we have failed to initialize our connections.
      std::cout << "exception thrown: aborting" << std::endl;
    }
    for (std::size_t i = 0; i < 20 && results.size_approx() > 0; ++i) {
      this_thread::sleep_for(1s);
      std::cout << "waiting for: " << results.size_approx() << std::endl;
      std::cout.flush();
      process_results(results, 1ms, now() + 100s, start_time);
    }
    output_file.flush();
  }
  template <typename time>
  void launcher_loop(moodycamel::ConcurrentQueue<
                         std::future<std::unique_ptr<run_result>>> &results,
                     time start_time, std::size_t parallel_factor) {
    using namespace server;
    using namespace pgsql;
    using namespace mtl;
    using namespace std;
    using namespace chrono;
    using namespace mutils;
    using namespace moodycamel;
    auto stop_time = start_time + params.test_duration;
    auto next_event_time = schedule_event(start_time, parallel_factor);
    auto next_desired_delay = next_event_time - now();
    auto &client_queue = this->client_queue;
    unsigned short choose_logging{0};
    while (now() < stop_time) {
      choose_logging = ((choose_logging + 1) % params.log_every_n);
      // always at least enqueue one client if needed
      if (number_enqueued_clients <
          params.total_clients_at(now() - start_time)) {
        push_client();
      }
      auto effective_delay = next_event_time - now();
      // work done, wait until event launch
      auto slept_for = next_event_time - now();
      this_thread::sleep_for(slept_for);
      auto this_event_time = next_event_time;
      auto desired_delay = next_desired_delay;
      // schedule next event
      next_event_time = schedule_event(start_time, parallel_factor);
      next_desired_delay = next_event_time - now();
      // try and handle this event (hopefully before it's time for the next one)
      std::unique_ptr<client> client_p;
      client_queue.wait_dequeue(client_p);
      bool log_this = (choose_logging == 0);
      auto fut = tp.push([
        slept_for,
        log_this,
        desired_delay,
        effective_delay,
        this_event_time,
        &client_queue,
        client_ptr = client_p.release()
      ](int) {
        std::unique_ptr<run_result> ret;
        if (log_this) {
					ret.reset(new run_result());
          ret->desired_delay = duration_cast<microseconds>(desired_delay);
          ret->effective_delay = duration_cast<microseconds>(effective_delay);
          ret->slept_for = duration_cast<microseconds>(slept_for);
          ret->start_time = this_event_time;
        }
        try {
          client_ptr->client_action(ret);
          client_queue.enqueue(std::unique_ptr<client>(client_ptr));
        } catch (const ProtocolException &) {
					if (!ret && log_this) ret.reset(new run_result());
					if (ret){
						// record this here and in simple_txn_test so
						// overhead of re-enqueueing client is not included.
						ret->stop_time = high_resolution_clock::now();
						ret->is_protocol_error = true;
					}
          --client_ptr->t.number_enqueued_clients;
        }
        return ret;
      });
      if (log_this)
        results.enqueue(std::move(fut));
    }
  }
};
}
