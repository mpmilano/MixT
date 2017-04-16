#pragma once
#include "configuration_params.hpp"
#include "run_result.hpp"
#include "relay_connections_pool.hpp"
#include "test_client.hpp"
#include "blockingconcurrentqueue.h"
#include "ctpl_stl.h"

namespace myria{

struct test{
	configuration_parameters params;

#ifndef NOPOOL
	pgsql::SQLConnectionPool<pgsql::Level::strong> spool;
	pgsql::SQLConnectionPool<pgsql::Level::causal> cpool;
#else
	const std::string spool{mutils::string_of_ip(params.strong_ip)};
	const std::string cpool{mutils::string_of_ip(params.causal_ip)};
#endif
	relay_connections_pool causal_connections{params,params.causal_ip, params.causal_relay_port,(int)params.max_clients()};
	relay_connections_pool strong_connections{params,params.strong_ip, params.strong_relay_port,(int)params.max_clients()};

	moodycamel::BlockingConcurrentQueue<std::unique_ptr<client> > client_queue;
	std::atomic<std::size_t> number_enqueued_clients{0};
	void push_client(){
		client_queue.enqueue(std::make_unique<client>(
													 *this,spool,cpool,
													 strong_connections.weakspawn(),causal_connections.weakspawn()));
		++number_enqueued_clients;
	}
	ctpl::thread_pool tp{(int)params.max_clients()/2};

	test(configuration_parameters params)
		:params(params){
		output_file << params << std::endl;
		for (std::size_t i = 0; i < params.starting_num_clients; ++i){
			push_client();
		}
	}

	auto now(){
		return std::chrono::high_resolution_clock::now();
	}
	
	template<typename Time>
	auto schedule_event(const Time& start_time, std::size_t parallel_factor){
		return now() + getArrivalInterval(params.current_arrival_rate(now() - start_time) / parallel_factor);
	}

	template<typename timeout, typename time>
	void process_results(moodycamel::ConcurrentQueue<std::future<run_result> > &results,
											 moodycamel::ConcurrentQueue<run_result> &pending_io,
											 timeout delay, time next_event_time){
		std::future<run_result> it;
		while (next_event_time > now() && results.try_dequeue(it)){
			if (it.wait_for(delay) != std::future_status::timeout){
				try {
					pending_io.enqueue(it.get());
				}
				catch (...){
					run_result r;
					r.is_fatal_error = true;
					pending_io.enqueue(r);
					--number_enqueued_clients;
				}
			}
			else {
				results.enqueue(std::move(it));
			}
		}
	}

	std::mutex output_file_lock;
	std::ofstream output_file{params.output_file};

	template<typename time>
	void print_result(const time &start_time, const run_result &r){
		std::unique_lock<std::mutex> flock{output_file_lock};
		r.print(start_time, output_file);
	}
	void run_test(){
		using namespace server;
		using namespace pgsql;
		using namespace mtl;
		using namespace std;
		using namespace chrono;
		using namespace mutils;
		using namespace moodycamel;
		std::size_t parallel_factor = 8;
		ConcurrentQueue<std::future<run_result> > results;
		ConcurrentQueue<run_result> pending_io;
		auto start_time = now();
		std::atomic<DECT(start_time)> last_log_write_time;
		last_log_write_time.store(start_time);
		std::atomic<std::size_t> event_count{0};
		try {
			std::vector<std::future<void> > launch_threads;
			for (std::size_t i =0 ; i < parallel_factor; ++i){
				launch_threads.emplace_back(std::async(launch::async, [&]{
							this->launcher_loop(results,pending_io,start_time,last_log_write_time,event_count,parallel_factor);							
						}));
			}
			for (auto &e : launch_threads){
				e.get();
			}

		} catch (...){
			//Looks like we have failed to initialize our connections.
			std::cout << "exception thrown: aborting" << std::endl;
		}
		for (std::size_t i = 0; i < 20 && results.size_approx() > 0; ++i){
			this_thread::sleep_for(1s);
			std::cout << "waiting for: " << results.size_approx();
			process_results(results,pending_io,1ms,now()+100s);
		}
		{
			run_result res;
			while (pending_io.try_dequeue(res)){
				print_result(start_time,res);
			}
		}
		output_file.flush();
	}
	template<typename time>
	void launcher_loop(moodycamel::ConcurrentQueue<std::future<run_result> > &results,
										 moodycamel::ConcurrentQueue<run_result> &pending_io,
										 time start_time, std::atomic<time> &last_log_write_time,
										 std::atomic<std::size_t> &event_count,
										 std::size_t parallel_factor){
		using namespace server;
		using namespace pgsql;
		using namespace mtl;
		using namespace std;
		using namespace chrono;
		using namespace mutils;
		using namespace moodycamel;
		auto stop_time = start_time + params.test_duration;
		auto next_event_time = schedule_event(start_time,parallel_factor);
		auto &client_queue = this->client_queue;
		for (; now() < stop_time; ++event_count){
			//always at least enqueue one client if needed
			if (number_enqueued_clients < params.total_clients_at(now() - start_time)){
					push_client();
			}
			if (next_event_time > now()) {
				//do some work in the meantime
				//increase client pool based on elapsed time
				while (number_enqueued_clients < params.total_clients_at(now() - start_time)
							 && next_event_time > now()){
					push_client();
				}
				//get partial results out of results list
				process_results(results,pending_io,0s,next_event_time);
				//rarely, try and print stuff
				if (event_count % 500 == 0) {
					while(next_event_time > now()){
						run_result result;
						if (pending_io.try_dequeue(result)){
							print_result(start_time,result);
						}
					}
					output_file.flush();
					last_log_write_time = now();
				}
			}
			//if it's been forever since our last log write, write the log *now*
			if (now() > (params.log_delay_tolerance + last_log_write_time.load())){
				std::cout << "log writer stalled, flushing now" << std::endl;
				process_results(results,pending_io,0s,1s + now());
				{
					run_result result;
					while (pending_io.try_dequeue(result)){
						print_result(start_time,result);
					}
				}
				output_file.flush();
				last_log_write_time = now();
			}
			
			//work done, wait until event launch
			auto slept_for = next_event_time - now();
			this_thread::sleep_for(slept_for);
			auto this_event_time = next_event_time;
			//schedule next event
			next_event_time = schedule_event(start_time,parallel_factor);
			//try and handle this event (hopefully before it's time for the next one)
			std::unique_ptr<client> client_p;
			client_queue.wait_dequeue(client_p);
			results.enqueue(tp.push([slept_for,this_event_time,&client_queue,
																		client_ptr = client_p.release()](int){
						run_result ret;
						ret.slept_for = duration_cast<microseconds>(slept_for);
						ret.start_time = this_event_time;
						try { 
							client_ptr->client_action(ret);
							client_queue.enqueue(std::unique_ptr<client>(client_ptr));
						}catch(const ProtocolException&){
							//record this here and in simple_txn_test so
							//overhead of re-enqueueing client is not included.
							ret.stop_time = high_resolution_clock::now();
							ret.is_protocol_error = true;
							--client_ptr->t.number_enqueued_clients;
						}
						return ret;
					}));
		}
	}
};
}
