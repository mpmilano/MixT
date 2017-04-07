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
	std::size_t number_enqueued_clients{0};
	void push_client(){
		client_queue.enqueue(std::make_unique<client>(
													 *this,spool,cpool,
													 strong_connections.weakspawn(),causal_connections.weakspawn()));
		++number_enqueued_clients;
	}
	ctpl::thread_pool tp{(int)params.max_clients()/2};

	test(configuration_parameters params)
		:params(params){
		for (std::size_t i = 0; i < params.starting_num_clients; ++i){
			push_client();
		}
	}

	auto now(){
		return std::chrono::high_resolution_clock::now();
	}
	
	template<typename Time>
	auto schedule_event(const Time& start_time){
		return now() + getArrivalInterval(params.current_arrival_rate(now() - start_time));
	}

	template<typename timeout, typename time>
	void process_results(std::list<std::future<run_result> > &results, std::list<run_result> &pending_io,
											 timeout delay, time next_event_time){
		for (auto iter = results.begin(); iter != results.end() && next_event_time > now(); ++iter){
			if (iter->wait_for(delay) != std::future_status::timeout){
				pending_io.emplace_back(iter->get());
				iter = results.erase(iter);
				--iter; //will increment on next loop execution
			}
		}
	}

	void print_result(const run_result &){
	}
	
	void run_test(){
		using namespace server;
		using namespace pgsql;
		using namespace mtl;
		using namespace std;
		using namespace chrono;
		using namespace mutils;
		auto &client_queue = this->client_queue;
		std::list<std::future<run_result> > results;
		std::list<run_result> pending_io;
		auto start_time = now();
		auto stop_time = start_time + params.test_duration;
		
		//schedule next event
		auto next_event_time = schedule_event(start_time);
		std::size_t event_count{0};
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
				if (event_count % 50 == 0) {
					while(next_event_time > now()){
						auto result = pending_io.front();
						pending_io.erase(pending_io.begin());
						print_result(result);
					}
				}
			}
			//work done, wait until event launch
			this_thread::sleep_for(now() - next_event_time);
			auto this_event_time = next_event_time;
			//schedule next event
			next_event_time = schedule_event(start_time);
			//try and handle this event (hopefully before it's time for the next one)
			std::unique_ptr<client> client_p;
			client_queue.wait_dequeue(client_p);
			results.emplace_back(tp.push([this_event_time,&client_queue,client_ptr = client_p.release()](int){
						auto ret = client_ptr->client_action(this_event_time);
						client_queue.enqueue(std::unique_ptr<client>(client_ptr));
						return ret;
					}));
		}
		while (results.size() > 0){
			std::cout << "waiting for: " << results.size();
			process_results(results,pending_io,1ms,now()+100s);
		}
		for (auto &res : pending_io){
			print_result(res);
		}
	}
};
}
