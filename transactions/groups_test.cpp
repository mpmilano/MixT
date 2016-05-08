#include "launch_test.hpp"
#include <chrono>
#include "GoogleGroups.hpp"


using namespace myria;
using namespace mtl;
using namespace std;
using namespace chrono;
using namespace mutils;
using namespace tracker;

constexpr const double write_percent = WRITE_PERCENT;
constexpr const double strong_percent = STRONG_PERCENT;

template<typename T>
struct causal_newobject{
    TrackerMem &grm;
	SQLMem &sm;
    auto newobj_f(std::unique_ptr<VMObjectLog> &log, int name) {
        auto &grm = this->grm;
		auto &sm = this->sm;
		using ret_t = std::decay_t<
			decltype(sm.sc.inst(0).
					 template newObject<HandleAccess::all>(grm.trk,mke_p<TransactionContext>(),name,std::declval<T>()))>;
        newObject_f<ret_t> ret = [&grm,&log,&sm,name](const T& t) -> ret_t {
            auto trans = start_transaction(log,
                                           grm.trk,
                                           sm.ss.inst(get_strong_ip()),
                                           sm.sc.inst(0));
            trans->commit_on_delete = true;
            return sm.sc.inst(0).
				template newObject<HandleAccess::all>(grm.trk, trans.get(),name,t);
        };
		return ret;
    }
};

template<typename T>
struct causal_newset{
    TrackerMem &grm;
	SQLMem &sm;
    newObject_f<typename remote_set<Level::causal,T>::p > newobj_f(std::unique_ptr<VMObjectLog> &log, int name) {
        auto &grm = this->grm;
		auto &sm = this->sm;
        return [&grm,&log,&sm,name](const std::set<T>& t){
            auto trans = start_transaction(log,
                                           grm.trk,
                                           sm.ss.inst(get_strong_ip()),
                                           sm.sc.inst(0));
            trans->commit_on_delete = true;
            return sm.sc.inst(0).
                    template newObject<HandleAccess::all>(grm.trk, trans.get(),name,t);
        };
    }
};

template<typename T>
struct strong_newobject{
    TrackerMem &grm;
	SQLMem &sm;
    auto newobj_f(std::unique_ptr<VMObjectLog> &log, int name) {
        auto &grm = this->grm;
		auto &sm = this->sm;
		using ret_t = decltype(sm.ss.inst(get_strong_ip()).
							   template newObject<HandleAccess::all>(grm.trk,mke_p<TransactionContext>(),name,std::declval<T>()));
        newObject_f<ret_t> ret = [&grm,&log,&sm,name](const T& t) -> ret_t{
            auto trans = start_transaction(log,
                                           grm.trk,
                                           sm.ss.inst(get_strong_ip()),
                                           sm.sc.inst(0));
            trans->commit_on_delete = true;
            return sm.ss.inst(get_strong_ip()).
                    template newObject<HandleAccess::all>(grm.trk, trans.get(),name,t);
        };
		return ret;
    }
};

int usernames {0};

struct TestParameters{
	using time_t = std::chrono::time_point<std::chrono::high_resolution_clock>;
	const time_t start_time{high_resolution_clock::now()};
	time_t last_rate_raise{start_time};
	Frequency current_rate{3000_Hz};
	constexpr static Frequency increase_factor = 20_Hz;
	constexpr static seconds increase_delay = 2s;
	constexpr static minutes test_stop_time = 7min;
	constexpr static double percent_writes = write_percent;
	constexpr static double percent_strong = strong_percent;
	
	constexpr static Name min_name = 17;
	constexpr static Name max_name = 400000-2;
	constexpr static double room_name_param = 0.5;
	
	struct GroupRemember{
		
        TrackerMem transaction_metadata;
		const std::size_t username;
		
		TrackerMem& tracker_mem(){ return transaction_metadata;}
		
        GroupRemember(int id)
            :transaction_metadata(id),username(++usernames){}
	};
	
	struct TestArguments{
        decltype(elapsed_time()) start_time;
        std::size_t rooms_index;
		std::size_t users_index;
	};
	
	static_assert(std::is_pod<TestArguments>::value, "Error: need POD for serialization");
	
	using PreparedTest = PreparedTest<GroupRemember,TestArguments>;
	using Pool = typename PreparedTest::Pool;
	
	//ids between 15 and 400000 should work (as long as they're not ints)
	static std::size_t get_name() {
		auto ret = get_zipfian_value(max_name - min_name,room_name_param);
		if (ret > (max_name - min_name)) {
			std::cerr << "Name out of range! Trying again" << std::endl;
			return get_name();
		}
		else return ret + min_name;
	}

/*	static std::size_t get_user_name(){
		return (int_rand() % (max_name - min_name)) + min_name;
		}/*/

        pair<int,TestArguments> choose_action(Pool&) const {
                bool do_write = better_rand() < percent_writes;

				
				TestArguments ta;
				ta.rooms_index = get_name();
				ta.start_time = elapsed_time();
				ta.users_index = get_name();
				
                //post message
                if (do_write) return pair<int,TestArguments>(0,ta);
                //check messages
                else return pair<int,TestArguments>(1,ta);
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
                pair<int,TestArguments> (*choose) (TestParameters&,Pool&) = method_to_fun(choose_action,Pool&);
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
constexpr Name TestParameters::min_name;
constexpr Name TestParameters::max_name;

int main(){
	ofstream logFile;
	const auto log_name = [](){
		auto pid = getpid();
		return std::string("/tmp/MyriaStore-output-") + std::to_string(pid);
	}();
	logFile.open(log_name);

	//initialize DB state
	//user::mke(causal_newobject<user>{gm.tracker_mem(),sm}.newobj_f(log,15),user::inbox_t::mke(causal_newset<typename post::p>{gm.tracker_mem(),sm}.newobj_f(log,16)) );
	//MemberList::mke(strong_newobject<MemberList>{gm.tracker_mem(),sm}.newobj_f(log,17));
	

	//auto prof = VMProfiler::startProfiling();
	//auto longpause = prof->pause();
	auto logger = build_VMObjectLogger();
	auto global_log = logger->template beginStruct<LoggedStructs::globals>();
	std::cout << "hello world from VM "<< my_unique_id << " in group " << CAUSAL_GROUP << std::endl;
	std::cout << "connecting to " << string_of_ip(get_strong_ip()) << std::endl;

	using TestArguments = TestParameters::TestArguments;
	
	const std::vector<typename TestParameters::Pool::action_fp> actions{{
                //post message
		[](int tid, shared_ptr<SQLMem> smem, int uid, std::shared_ptr<typename TestParameters::GroupRemember> gmem, TestArguments args){
                        std::unique_ptr<VMObjectLog> log = gmem->
                                transaction_metadata.log_builder->
                                template beginStruct<LoggedStructs::log>();
                        log->addField(LogFields::submit_time,duration_cast<milliseconds>(args.start_time).count());
                        log->addField(LogFields::run_time,duration_cast<milliseconds>(elapsed_time()).count());

						room{MemberList::p{}}.
							add_post(log,gmem->transaction_metadata.trk,post::p{});
                        log->addField(LogFields::done_time,duration_cast<milliseconds>(elapsed_time()).count());
                        return log->single();
                },
			//read inbox
			[](int tid, shared_ptr<SQLMem> smem, int uid, std::shared_ptr<typename TestParameters::GroupRemember> gmem, TestArguments args){
                std::unique_ptr<VMObjectLog> log = gmem->
					transaction_metadata.log_builder->
					template beginStruct<LoggedStructs::log>();
				std::unique_ptr<VMObjectLog> log2 = gmem->
					transaction_metadata.log_builder->
					template beginStruct<LoggedStructs::log>();
                log->addField(LogFields::submit_time,duration_cast<milliseconds>(args.start_time).count());
                log->addField(LogFields::run_time,duration_cast<milliseconds>(elapsed_time()).count());
				log2->addField(LogFields::submit_time,duration_cast<milliseconds>(args.start_time).count());
				log2->addField(LogFields::run_time,duration_cast<milliseconds>(elapsed_time()).count());
				auto &trk = gmem->transaction_metadata.trk;
				user::posts_p(log,trk,smem->sc.inst(0).template existingObject<HandleAccess::all,user>(log,args.users_index));
                log->addField(LogFields::done_time,duration_cast<milliseconds>(elapsed_time()).count());
				log2->addField(LogFields::done_time,duration_cast<milliseconds>(elapsed_time()).count());
				//double log, because two reads.
                return log->single() + "\n" + log2->single();
        }

        }};

	auto gm = std::make_shared<TestParameters::GroupRemember>(15);
	//std::unique_ptr<VMObjectLog> log{gm.tracker_mem().log_builder->template beginStruct<LoggedStructs::log>().release()};
	auto sm = std::make_shared<SQLMem>(gm->tracker_mem());
	PreparedTest<typename TestParameters::GroupRemember,int>::pool_mem_init(15,sm,15,gm);
	TestArguments ta;
	ta.rooms_index = TestParameters::get_name();
	ta.start_time = elapsed_time();
	ta.users_index = TestParameters::get_name();
	actions.at(0)(0,sm,0,gm,ta);
	actions.at(1)(0,sm,0,gm,ta);
	//exit(0);
	
	typename TestParameters::PreparedTest launcher{
		num_processes,actions};
	
	std::cout << "beginning subtask generation loop" << std::endl;

	TestParameters params{*global_log};
	const std::string results = params.run_tests(launcher);
		
	//resume profiling
	
	global_log->addField(GlobalsFields::final_completion_time,
						 duration_cast<milliseconds>(elapsed_time()).count());

	logFile << logger->declarations() << endl;

	logFile << global_log->single() << endl;
	logFile << results;
}
