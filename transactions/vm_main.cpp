
#include "ProcessPool.hpp"
#include "ThreadPool.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <pqxx/pqxx>
#include "SQLStore.hpp"
#include "FinalHeader.hpp"
#include "Ostreams.hpp"
#include "tuple_extras.hpp"
#include "Basics.hpp"
#include <sys/types.h>
#include <chrono>
#include <cmath>
#include <unistd.h>
#include "ProcessPool.hpp"
#include "Hertz.hpp"
#include "ObjectBuilder.hpp"
#include "launch_test.hpp"
//*/
#include "Transaction_macros.hpp"

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

constexpr int name_max = 478446;

using fake_time = unsigned long long;

namespace synth_test {

	int get_name_read(double alpha){
		constexpr int max = name_max;
		auto ret = get_zipfian_value(max,alpha);
		if (ret > (max + 14)) {
			std::cerr << "Name out of range! Trying again" << std::endl;
			return get_name_read(alpha);
		}
		else return ret + 14;
	}
	
	int get_name_write(){
        return 14 + int_rand() % 478446;
	}

	template<typename Hndl>
	using oper_f = void (*) (unique_ptr<VMObjectLog>& ,
							 Tracker &, Hndl );

	auto log_start(std::shared_ptr<TrackerMem> mem, unique_ptr<VMObjectLog>& log_messages, fake_time _start_time){
		assert(mem);
		log_messages = mem->log_builder->template beginStruct<LoggedStructs::log>();
		microseconds start_time(_start_time);
		auto run_time = elapsed_time();
		
		log_messages->addField(LogFields::submit_time,
							   duration_cast<milliseconds>(start_time).count());
		log_messages->addField(LogFields::run_time,
							   duration_cast<milliseconds>(run_time).count());
                assert(log_messages);
                return start_time;
	}

	template<typename Strong, typename Causal>
	void store_asserts(Strong &strong, Causal& causal, Tracker &trk){
		assert(!strong.in_transaction());
		assert(!causal.in_transaction());
		assert(!trk.get_StrongStore().in_transaction());
	}
	
	template<typename Hndl>
	void perform_increment(unique_ptr<VMObjectLog>& log_messages,
										 Tracker &trk, Hndl hndl){
		TRANSACTION(log_messages,trk,hndl,
					do_op<RegisteredOperations::Increment>(hndl)
			)//*/
			log_messages->addField(
				LogFields::is_write,true);
	}

	template<typename Hndl>
	void perform_read(unique_ptr<VMObjectLog>& log_messages,
							 Tracker &trk, Hndl hndl){
		TRANSACTION(log_messages,
					trk,hndl,
					let_remote(tmp) = hndl IN(mtl_ignore(tmp))
			);

		struct tmptest{ int a;};
		Handle<Level::strong,HandleAccess::all, tmptest> j;
		
		log_messages->addField(
			LogFields::is_read,true);
		
	}

	template<typename Hndl>
	void perform_operation(unique_ptr<VMObjectLog>& log_messages,
								  Tracker &trk, Hndl hndl, oper_f<Hndl> oper){
		try{ 
			for(int tmp2 = 0; tmp2 < 10; ++tmp2){
				try{
					oper(log_messages,trk,hndl);
					auto end = elapsed_time();
					log_messages->addField(LogFields::done_time,
										   duration_cast<milliseconds>(end).count());
					break;
				}
				catch(const SerializationFailure& sf){
					auto end = elapsed_time();
					log_messages->addField(LogFields::done_time,
										   duration_cast<milliseconds>(end).count());
					log_messages->addField(LogFields::is_serialization_error,true);
					continue;
				}
			}
		}
		catch(pqxx::pqxx_exception &e){
			log_messages->addField(LogFields::pqxx_failure,true);
			log_messages->addField(LogFields::pqxx_failure_string, std::string(e.base().what()));
		}
	}

	std::string perform_strong_increment(int, std::shared_ptr<SQLMem> sm, int, std::shared_ptr<TrackerMem> tm, 
										 fake_time _start_time){
		auto name = get_name_write();
		std::unique_ptr<VMObjectLog> log_messages;
		log_start(tm,log_messages,_start_time);
		SQLStore<Level::strong> &strong = sm->ss.inst_strong(get_strong_ip());
		SQLStore<Level::causal> &causal = sm->sc.inst_causal(0);
		auto &trk = tm->trk;
		store_asserts(strong,causal,trk);
		perform_operation(log_messages, trk,
						  strong.template existingObject<HandleAccess::all,int>(log_messages,name),
						  perform_increment
			);
		log_messages->addField(LogFields::is_strong,true);
		return log_messages->single();
	}

	std::string perform_causal_increment(int,  std::shared_ptr<SQLMem> sm, int, std::shared_ptr<TrackerMem> tm,
										 fake_time _start_time){
		auto name = get_name_write();
		std::unique_ptr<VMObjectLog> log_messages;
		log_start(tm,log_messages,_start_time);
		assert(log_messages);
		SQLStore<Level::strong> &strong = sm->ss.inst_strong(get_strong_ip());
		SQLStore<Level::causal> &causal = sm->sc.inst_causal(0);
		auto &trk = tm->trk;
		store_asserts(strong,causal,trk);
		perform_operation(log_messages,trk,
						  causal.template existingObject<HandleAccess::all,int>(log_messages,name),
						  perform_increment
			);
		log_messages->addField(LogFields::is_causal,true);
		return log_messages->single();
		
	}

	std::string perform_strong_read(int,  std::shared_ptr<SQLMem> sm, int, std::shared_ptr<TrackerMem> tm,
										 fake_time _start_time){
		auto name = get_name_read(0.5);
		std::unique_ptr<VMObjectLog> log_messages;
		log_start(tm,log_messages,_start_time);
		SQLStore<Level::strong> &strong = sm->ss.inst_strong(get_strong_ip());
		SQLStore<Level::causal> &causal = sm->sc.inst_causal(0);
		auto &trk = tm->trk;
		store_asserts(strong,causal,trk);
		perform_operation(log_messages, trk,
						  strong.template existingObject<HandleAccess::all,int>(log_messages,name),
						  perform_read
			);
		log_messages->addField(LogFields::is_strong,true);
		return log_messages->single();
	}

	std::string perform_causal_read(int,  std::shared_ptr<SQLMem> sm, int, std::shared_ptr<TrackerMem> tm,
										 fake_time _start_time){
		auto name = get_name_read(0.5);
		std::unique_ptr<VMObjectLog> log_messages;
		log_start(tm,log_messages,_start_time);
		SQLStore<Level::strong> &strong = sm->ss.inst_strong(get_strong_ip());
		SQLStore<Level::causal> &causal = sm->sc.inst_causal(0);
		auto &trk = tm->trk;
		store_asserts(strong,causal,trk);
		perform_operation(log_messages,trk,
						  causal.template existingObject<HandleAccess::all,int>(log_messages,name),
						  perform_read
			);
		log_messages->addField(LogFields::is_causal,true);
		return log_messages->single();

	}

	struct TestParameters{
		using time_t = std::chrono::time_point<std::chrono::high_resolution_clock>;
		const time_t start_time{high_resolution_clock::now()};
		time_t last_rate_raise{start_time};
		Frequency current_rate{5000_Hz};
		constexpr static Frequency increase_factor = 20_Hz;
		constexpr static seconds increase_delay = 2s;
		constexpr static minutes test_stop_time = 7min;
		constexpr static double percent_writes = write_percent;
		constexpr static double percent_strong = strong_percent;
		using PreparedTest = PreparedTest<TrackerMem,fake_time>;
		using Pool = typename PreparedTest::Pool;
		
		pair<int,fake_time> choose_action(Pool&) const {
			bool do_write = better_rand() < percent_writes;
			bool is_strong = (better_rand() < percent_strong || !causal_enabled) && strong_enabled;
			if (do_write && is_strong) return pair<int,fake_time>(0,micros(elapsed_time()));
			if (do_write && !is_strong) return pair<int,fake_time>(1,micros(elapsed_time()));
			if (!do_write && is_strong) return pair<int,fake_time>(2,micros(elapsed_time()));
			if (!do_write && !is_strong) return pair<int,fake_time>(3,micros(elapsed_time()));
			assert(false);
		}

		bool stop (Pool&) const {
			return (high_resolution_clock::now() - start_time) >= test_stop_time;
		};

		milliseconds delay(Pool& p){
			if (high_resolution_clock::now() - last_rate_raise > increase_delay){
				current_rate += increase_factor;
				last_rate_raise = high_resolution_clock::now();
			}
			//there should always be 10 request/second/client
			p.set_mem_to(current_rate.hertz / 10);
			return getArrivalInterval(current_rate);
		}

#define method_to_fun(foo,Arg) [](auto& x, Arg y){return x.foo(y);}
		std::string run_tests(PreparedTest& launcher){
			bool (*stop) (TestParameters&,Pool&) = method_to_fun(stop,Pool&);
			pair<int,fake_time> (*choose) (TestParameters&,Pool&) = method_to_fun(choose_action,Pool&);
			milliseconds (*delay) (TestParameters&,Pool&) = method_to_fun(delay,Pool&);
			auto ret = launcher.run_tests(*this,stop,choose,delay);

			global_log.addField(GlobalsFields::request_frequency_final,current_rate);
			return ret;
		}
		
		abs_StructBuilder &global_log;
		
		TestParameters(decltype(global_log) &gl):global_log(gl){
			global_log.addField(GlobalsFields::request_frequency,current_rate);
			global_log.addField(GlobalsFields::request_frequency_step,increase_factor);
		}
	};
	constexpr Frequency TestParameters::increase_factor;
	constexpr seconds TestParameters::increase_delay;
	constexpr minutes TestParameters::test_stop_time;
	constexpr double TestParameters::percent_writes;
	constexpr double TestParameters::percent_strong;


}

int main(){

	std::cout << "In configuration; " << (causal_enabled ? "with causal" : " with only strong" ) << std::endl;
	ofstream logFile;
	logFile.open(log_name);
	//auto prof = VMProfiler::startProfiling();
	//auto longpause = prof->pause();
	auto logger = build_VMObjectLogger();
	auto global_log = logger->template beginStruct<LoggedStructs::globals>();
	std::cout << "hello world from VM "<< my_unique_id << " in group " << CAUSAL_GROUP << std::endl;
	std::cout << "connecting to " << string_of_ip(get_strong_ip()) << std::endl;

	using pool_fun_t = typename synth_test::TestParameters::PreparedTest::action_t;

	vector<pool_fun_t> vec{{
			synth_test::perform_strong_increment,
				synth_test::perform_causal_increment,
				synth_test::perform_strong_read,
				synth_test::perform_causal_read
				}};
	
	typename synth_test::TestParameters::PreparedTest launcher{
		num_processes,vec};
	
	std::cout << "beginning subtask generation loop" << std::endl;

	synth_test::TestParameters params{*global_log};
	const std::string results = params.run_tests(launcher);
		
	//resume profiling
	
	global_log->addField(GlobalsFields::final_completion_time,
						 duration_cast<milliseconds>(elapsed_time()).count());

	logFile << logger->declarations() << endl;

	logFile << global_log->single() << endl;
	logFile << results;
	

}
