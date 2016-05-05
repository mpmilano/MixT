#include "GoogleGroups.hpp"
#include "launch_test.hpp"
#include <chrono>

using namespace myria;
using namespace mtl;
using namespace std;
using namespace chrono;
using namespace mutils;
using namespace tracker;

struct TestArguments{
        microseconds start_time;
        std::size_t users_index;
        std::size_t rooms_index;
};

static_assert(std::is_pod<TestArguments>::value, "Error: need POD for serialization");

template<typename T>
struct causal_newobject{
    GroupRemember &grm;
    newObject_f<Handle<Level::causal,HandleAccess::all,T> > newobj_f(std::unique_ptr<VMObjectLog> &log, int) {
        auto &grm = this->grm;
        return [&grm](const T& t){
            auto trans = start_transaction(log,
                                           grm.transaction_metadata.trk,
                                           grm.transaction_metadata.ss.inst(get_strong_ip()),
                                           grm.transaction_metadata.sc.inst(0));
            trans->commit_on_delete = true;
            return grm.transaction_metadata.sc.inst(0).
                    template newObject<HandleAccess::all>(grm.transaction_metadata.trk, trans.get(),t);
        };
    }
};

template<typename T>
struct causal_newset{
    GroupRemember &grm;
    newObject_f<typename remote_set::p<Level::causal,T> > newobj_f(std::unique_ptr<VMObjectLog> &log, int) {
        auto &grm = this->grm;
        return [&grm](const std::set<T>& t){
            auto trans = start_transaction(log,
                                           grm.transaction_metadata.trk,
                                           grm.transaction_metadata.ss.inst(get_strong_ip()),
                                           grm.transaction_metadata.sc.inst(0));
            trans->commit_on_delete = true;
            return grm.transaction_metadata.sc.inst(0).
                    template newObject<HandleAccess::all>(grm.transaction_metadata.trk, trans.get(),t);
        };
    }
};

template<typename T>
struct strong_newobject{
    GroupRemember &grm;
    newObject_f<Handle<Level::strong,HandleAccess::all,T> > newobj_f(std::unique_ptr<VMObjectLog> &log, int) {
        auto &grm = this->grm;
        return [&grm](const T& t){
            auto trans = start_transaction(log,
                                           grm.transaction_metadata.trk,
                                           grm.transaction_metadata.ss.inst(get_strong_ip()),
                                           grm.transaction_metadata.sc.inst(0));
            trans->commit_on_delete = true;
            return grm.transaction_metadata.ss.inst(get_strong_ip()).
                    template newObject<HandleAccess::all>(grm.transaction_metadata.trk, trans.get(),t);
        };
    }
};

struct GroupRemember{

        TrackerMem transaction_metadata;

        GroupRemember(int id)
            :transaction_metadata(id){}
};

struct TestParameters{
        using time_t = std::chrono::time_point<std::chrono::high_resolution_clock>;
        const time_t start_time{high_resolution_clock::now()};
        time_t last_rate_raise{start_time};
        Frequency current_rate{1000_Hz};
        constexpr static Frequency increase_factor = 20_Hz;
        constexpr static seconds increase_delay = 2s;
        constexpr static minutes test_stop_time = 7min;
        constexpr static double percent_writes = write_percent;
        constexpr static double percent_strong = strong_percent;
        using PreparedTest = PreparedTest<GroupRemember,TestArguments>;
        using Pool = typename PreparedTest::Pool;

        //ids between 15 and 400000 should work (as long as they're not ints)
        constexpr static Name min_name = 15;
        constexpr static Name max_name = 400000;
        static std::vector<user> users; //these are immutable once initialized
        static std::vector<room> rooms; //these are immutable once initialized

        pair<int,TestArguments> choose_action(Pool&) const {
                bool do_write = better_rand() < percent_writes;
                bool is_strong = (better_rand() < percent_strong || !causal_enabled) && strong_enabled;
                //join group
                if (do_write && is_strong) return pair<int,TestArguments>(1,micros(elapsed_time()));
                //post message
                else if (do_write && !is_strong) return pair<int,TestArguments>(0,micros(elapsed_time()));
                //check messages
                else return pair<int,TestArguments>(3,micros(elapsed_time()));
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
constexpr Name TestParameters::min_name;
constexpr Name TestParameters::max_name;

const std::vector<action_t> actions{{
                //post message
                [](std::shared_ptr<GroupRemember> mem, int tid, TestArguments args){
                        auto log = mem->
                                transaction_metadata.log_builder->
                                template beginStruct<LoggedStructs::log>();
                        log->addField(LogFields::submit_time,duration_cast<milliseconds>(args.start_time).count());
                        log->addField(LogFields::run_time,duraction_cast<milliseconds>(elapsed_time()).count());

                        mem->rooms.at(args.rooms_index).
                                add_post(log,mem->transaction_metadata.trk,
                                         post::mke(causal_newobject<post>{*mem}.newobj_f(log,tid),"test message"));
                        log->addField(LogFields::done_time,duration_cast<milliseconds>(elapsed_time()).count());
                        return log->single();
                },
        //join group
        [](std::shared_ptr<GroupRemember> mem, int tid, TestArguments args){
                auto log = mem->
                        transaction_metadata.log_builder->
                        template beginStruct<LoggedStructs::log>();
                log->addField(LogFields::submit_time,duration_cast<milliseconds>(args.start_time).count());
                log->addField(LogFields::run_time,duraction_cast<milliseconds>(elapsed_time()).count());
                mem->rooms.at(args.rooms_index).add_member(log,mem->transaction_metadata.trk,);
                log->addField(LogFields::done_time,duration_cast<milliseconds>(elapsed_time()).count());
                return log->single();
        },

        }};
