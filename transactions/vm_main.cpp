
#include "GloballyBackedExecutor.hpp"
#include <iostream>
#include <sys/resource.h>
#include <fstream>
#include <thread>
#include "SQLStore.hpp"
#include "FinalHeader.hpp"
#include "Ostreams.hpp"
#include "tuple_extras.hpp"
#include "Basics.hpp"
#include <sys/types.h>
#include <chrono>
#include <cmath>
#include <unistd.h>
#include "Hertz.hpp"
#include "ObjectBuilder.hpp"
#include "launch_test.hpp"
//*/
#include "transaction.hpp"
#include "transaction_macros.hpp"
#include "split_printer.hpp"

using namespace std;
using namespace chrono;
using namespace mutils;
using namespace myria;
using namespace mtl;
using namespace pgsql;
using namespace tracker;

#ifdef NO_USE_STRONG
constexpr bool strong_enabled = false;
#else
constexpr bool strong_enabled = true;
#endif

#ifdef NO_USE_CAUSAL
constexpr bool causal_enabled = false;
#else
constexpr bool causal_enabled = true;
#endif

constexpr const double write_percent = WRITE_PERCENT;
constexpr const double strong_percent = STRONG_PERCENT;

static_assert(causal_enabled || strong_enabled, "Error: do not disable both stores.");

const auto log_name = [](){
	auto pid = getpid();
	return std::string("/tmp/MyriaStore-output-") + std::to_string(pid);
}();

using fake_time = unsigned long long;

namespace synth_test {

	template<typename Hndl>
	using oper_f = void (*) (unique_ptr<VMObjectLog>& ,
													 DeserializationManager* , mutils::connection&,
													 Tracker &, Hndl );

	auto log_start(Mem& mem, unique_ptr<VMObjectLog>& log_messages, int name, fake_time _start_time){
		log_messages = mem.log_builder->template beginStruct<LoggedStructs::log>();
		microseconds start_time(_start_time);
		auto run_time = elapsed_time();
		
		log_messages->addField(LogFields::submit_time,
							   duration_cast<milliseconds>(start_time).count());
		log_messages->addField(LogFields::run_time,
							   duration_cast<milliseconds>(run_time).count());
		log_messages->addField(LogFields::item_name,name);
		assert(log_messages);
		return start_time;
	}

#ifndef NDEBUG
	template<typename Strong, typename Causal>
	void store_asserts(Strong &strong, Causal& causal, Tracker &){
		assert(!strong.in_transaction());
		assert(!causal.in_transaction());
	}
#endif
	
	template<typename Hndl>
	void perform_increment(unique_ptr<VMObjectLog>& log_messages,
												 DeserializationManager* dsm, mutils::connection& conn, Tracker &, Hndl hndl){
		constexpr auto trans = TRANSACTION(Hndl::label::int_id::value,let remote x = hndl in {x = x + 1})::WITH(hndl);
		trans.run_optimistic(dsm,conn,hndl);
			log_messages->addField(
				LogFields::is_write,true);
	}

	template<typename Hndl>
	void perform_read(unique_ptr<VMObjectLog>& log_messages,
										DeserializationManager* dsm, mutils::connection& conn, Tracker &, Hndl hndl){
		constexpr auto trans = TRANSACTION(150 + Hndl::label::int_id::value,let remote x = hndl in {})::WITH(hndl);
		trans.run_optimistic(dsm,conn,hndl);

#ifndef NDEBUG
		struct tmptest{ int a;};
		Handle<Label<pgsql::strong>,tmptest> j;
#endif
		
		log_messages->addField(
			LogFields::is_read,true);
		
	}

	template<typename Hndl>
	void perform_operation(unique_ptr<VMObjectLog>& log_messages,
												 DeserializationManager* dsm, mutils::connection& conn,
												 Tracker &trk, Hndl hndl, oper_f<Hndl> oper){
		try{ 
			for(int tmp2 = 0; tmp2 < 10; ++tmp2){
				try{
					oper(log_messages,dsm,conn,trk,hndl);
					auto end = elapsed_time();
					log_messages->addField(LogFields::done_time,
										   duration_cast<milliseconds>(end).count());
					break;
				}
				catch(const SerializationFailure& sf){
					auto end = elapsed_time();
					log_messages->addField(LogFields::done_time,
										   duration_cast<milliseconds>(end).count());
					//log_messages->addField(LogFields::is_serialization_error,true);
					log_messages->addField(LogFields::remote_failure_string, std::string(sf.what()) + ": serialization failure");
					continue;
				}
			}
		}
		catch(const SerializationFailure &e){
			log_messages->addField(LogFields::remote_failure_string, std::string(e.what()) + ": serialization failure");
		}
	}

	std::string perform_strong_increment(int, Mem& mem, fake_time _start_time){
		auto name = get_name_write();
		std::unique_ptr<VMObjectLog> log_messages;
		log_start(mem,log_messages,name,_start_time);
		SQLStore<Level::strong> &strong = mem.i->ss.inst();
		auto &trk = mem.trk;
#ifndef NDEBUG
		store_asserts(strong,mem.i->sc.inst(),trk);
#endif
		perform_operation(log_messages, mem.dsm(), mem.i->strong_connection, trk,
						  strong.template existingObject<int>(name),
						  perform_increment
			);
		log_messages->addField(LogFields::is_strong,true);
		return log_messages->single();
	}

	std::string perform_causal_increment(int,Mem& mem,fake_time _start_time){
		auto name = get_name_write();
		std::unique_ptr<VMObjectLog> log_messages;
		log_start(mem,log_messages,name,_start_time);
		assert(log_messages);
		SQLStore<Level::causal> &causal = mem.i->sc.inst();
		auto &trk = mem.trk;
#ifndef NDEBUG
		store_asserts(mem.i->ss.inst(),causal,trk);
#endif
		perform_operation(log_messages,mem.dsm(), mem.i->causal_connection, trk,
						  causal.template existingObject<int>(name),
						  perform_increment
			);
		log_messages->addField(LogFields::is_causal,true);
		return log_messages->single();
		
	}

	std::string perform_strong_read(int, Mem& mem, fake_time _start_time){
		auto name = get_name_read(0.5);
		std::unique_ptr<VMObjectLog> log_messages;
		log_start(mem,log_messages,name,_start_time);
		SQLStore<Level::strong> &strong = mem.i->ss.inst();
		auto &trk = mem.trk;
#ifndef NDEBUG
		store_asserts(strong,mem.i->sc.inst(),trk);
#endif
		perform_operation(log_messages, mem.dsm(), mem.i->strong_connection, trk,
						  strong.template existingObject<int>(name),
						  perform_read
			);
		log_messages->addField(LogFields::is_strong,true);
		return log_messages->single();
	}

	std::string perform_causal_read(int, Mem& mem, fake_time _start_time){
		auto name = get_name_read(0.5);
		std::unique_ptr<VMObjectLog> log_messages;
		log_start(mem,log_messages,name,_start_time);
		SQLStore<Level::causal> &causal = mem.i->sc.inst();
		auto &trk = mem.trk;
#ifndef NDEBUG
		store_asserts(mem.i->ss.inst(),causal,trk);
#endif
		perform_operation(log_messages,mem.dsm(), mem.i->causal_connection, trk,
						  causal.template existingObject<int>(name),
						  perform_read
			);
		log_messages->addField(LogFields::is_causal,true);
		return log_messages->single();

	}

	struct TestParameters{
		using time_t = std::chrono::time_point<std::chrono::high_resolution_clock>;
		const time_t start_time{high_resolution_clock::now()};
		time_t last_rate_raise{start_time};
		constexpr static Frequency rate_per_client{CLIENT_RATE};
		unsigned int num_clients = NUM_CLIENTS;
		constexpr static unsigned int increase_factor = INCREASE_BY;
		constexpr static auto increase_delay = INCREASE_DELAY;
		constexpr static auto test_stop_time = TEST_STOP_TIME;
		constexpr static double percent_writes = write_percent;
		constexpr static double percent_strong = strong_percent;
		using PreparedTest = PreparedTest<Mem,fake_time>;
		using Pool = typename PreparedTest::Pool;
		
		pair<int,fake_time> choose_action(Pool&) const {
			bool do_write = better_rand() < percent_writes;
			bool is_strong = (better_rand() < percent_strong || !causal_enabled) && strong_enabled;
			if (do_write && is_strong) return pair<int,fake_time>(0,micros(elapsed_time()));
			if (do_write && !is_strong) return pair<int,fake_time>(1,micros(elapsed_time()));
			if (!do_write && is_strong) return pair<int,fake_time>(2,micros(elapsed_time()));
			if (!do_write && !is_strong) return pair<int,fake_time>(3,micros(elapsed_time()));
			assert(false);
			struct dead_code{}; throw dead_code{};
		}

		bool stop (Pool&) const {
			return (high_resolution_clock::now() - start_time) >= test_stop_time;
		};

		milliseconds delay(Pool& p){
			if (high_resolution_clock::now() - last_rate_raise > increase_delay){
				num_clients += increase_factor;
				last_rate_raise = high_resolution_clock::now();
			}
			p.set_mem_to(num_clients);
			return getArrivalInterval(rate_per_client * num_clients);
		}

#define method_to_fun(foo,Arg) [](auto& x, Arg y){return x.foo(y);}
		
		std::string run_tests(PreparedTest& launcher){
			bool (*stop) (TestParameters&,Pool&) = method_to_fun(stop,Pool&);
			pair<int,fake_time> (*choose) (TestParameters&,Pool&) = method_to_fun(choose_action,Pool&);
			milliseconds (*delay) (TestParameters&,Pool&) = method_to_fun(delay,Pool&);
			auto ret = launcher.run_tests(*this,stop,choose,delay);

			global_log.addField(GlobalsFields::request_frequency_final,rate_per_client*num_clients);
			return ret;
		}
		
		abs_StructBuilder &global_log;
		
		TestParameters(decltype(global_log) &gl):global_log(gl){
			global_log.addField(GlobalsFields::request_frequency,rate_per_client * num_clients);
			global_log.addField(GlobalsFields::request_frequency_step,increase_factor);
		}
	};
	constexpr Frequency TestParameters::rate_per_client;
	constexpr unsigned int TestParameters::increase_factor;
	constexpr decltype(TestParameters::increase_delay) TestParameters::increase_delay;
	constexpr decltype(TestParameters::test_stop_time) TestParameters::test_stop_time;
	constexpr double TestParameters::percent_writes;
	constexpr double TestParameters::percent_strong;


}

int real_main(int strong_relay_ip, int strong_relay_port, int causal_relay_ip, int causal_relay_port){

	std::cout << "In configuration; " << (causal_enabled ? "with causal" : " with only strong" ) << std::endl;
	ofstream logFile;
	logFile.open(log_name);
	//auto prof = VMProfiler::startProfiling();
	//auto longpause = prof->pause();
	auto logger = build_VMObjectLogger();
	auto global_log = logger->template beginStruct<LoggedStructs::globals>();
	std::cout << "hello world from VM "<< my_unique_id << " in group " << CAUSAL_GROUP << std::endl;
	std::cout << "connecting to " << string_of_ip(get_strong_ip()) << " and " << string_of_ip(get_causal_ip()) << std::endl;

	using pool_fun_t = typename synth_test::TestParameters::PreparedTest::action_t;

	vector<pool_fun_t> vec{{
			synth_test::perform_strong_increment,
				synth_test::perform_causal_increment,
				synth_test::perform_strong_read,
				synth_test::perform_causal_read
				}};
	
	typename synth_test::TestParameters::PreparedTest
		launcher{vec,strong_relay_ip,strong_relay_port,MAX_THREADS/2,causal_relay_ip,causal_relay_port,MAX_THREADS/2};
	
	std::cout << "beginning subtask generation loop" << std::endl;

	synth_test::TestParameters params{*global_log};
	const std::string results = params.run_tests(launcher);
		
	//resume profiling
	
	global_log->addField(GlobalsFields::final_completion_time,
						 duration_cast<milliseconds>(elapsed_time()).count());

	logFile << logger->declarations() << endl;

	logFile << global_log->single() << endl;
	logFile << results;
	
	return 0;
}

int main(int whendebug(argc), char** argv){
	assert(argc > 4);
	const rlim_t kStackSize = 128 * 1024 * 1024;   // min stack size = 16 MB
	struct rlimit rl;
	int result;
	
	result = getrlimit(RLIMIT_STACK, &rl);
	if (result == 0)
	{
		if (rl.rlim_cur < kStackSize)
		{
			rl.rlim_cur = kStackSize;
			result = setrlimit(RLIMIT_STACK, &rl);
			if (result != 0)
			{
				fprintf(stderr, "setrlimit returned result = %d\n", result);
			}
		}
	}
	try {
		return real_main(mutils::decode_ip(argv[1]),atoi(argv[2]),mutils::decode_ip(argv[3]),atoi(argv[4]));
	}
	catch(const std::exception &e){
		std::cout << e.what() << std::endl;
	}
}
