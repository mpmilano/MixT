#pragma once

/*
  Example: Chat server. Users, rooms.  Room lists are linearizable; Room membership is linearizable.  Each user has an inbox, and each room also maintains a list of posts.  Posts, and the delivery thereof, are causal. When a user posts to a room, it's copied to each room-member in a single transaction.  This is a hybrid transaction; you need to read-validate the room-list at the end, and you also need to make sure the post shows up in all or none of the inboxes.  Users can't delete posts they make (though they can delete their own local things from their inbox), and users can join and leave rooms whenever. 

*/


//other examples: wiki edites?
//chat bigger example: pin messages is endorsement. Maybe for moderator elections?

//endorsements: include an endorsement that does a quorum read, and another which doesn't, for data-dependant things (like computation warranties use case).

#include <set>
#include <sstream>
#include "MTL.hpp"
#include "SQLStore.hpp"
//#include "TrackerTestingStore.hpp"
#include "RemoteCons.hpp"
#include "Ostreams.hpp"
#include "Operations.hpp"
#include "FinalHeader.hpp"//*/
#include "SerializationMacros.hpp"
#include "Transaction_macros.hpp"

template<typename p>
using newObject_f = const std::function<p (const typename p::stored_type&)>&;

#define default_build 	template<typename... T>					\
	static p mke( newObject_f<p> store_alloc, const T&... a){	\
		typename p::stored_type ret{a...};						\
		return store_alloc(ret);								\
	}

template<Level l, typename T2>
struct remote_set {
	using p = Handle<l,HandleAccess::all, std::set<T2>,
					 SupportedOperation<RegisteredOperations::Insert,SelfType> >;
	default_build
};

struct post : public ByteRepresentable{
	using p = Handle<Level::causal, HandleAccess::all, post>;
	std::string str;
	post(const std::string &s):str(s){}
	default_build
	DEFAULT_SERIALIZATION_SUPPORT(post,str)
		};

struct user : public ByteRepresentable {
	using p = Handle<Level::causal, HandleAccess::all, user>;
	typename remote_set<Level::causal, post::p>::p inbox;
	user(const decltype(inbox) &i):inbox(i){}
	
	default_build
	DEFAULT_SERIALIZATION_SUPPORT(user,inbox);


	std::set<post::p> posts() {
		auto start_transaction(std::unique_ptr<VMObjectLog> &log, tracker::Tracker &trk, Strong &strong, Causal &causal);
	}
		
};

/*std::ostream& operator<<(std::ostream &os, const user& u){
  return os << "user with inbox: " << u.inbox;
  }
*/

using MemberList = RemoteCons<user,Level::strong,Level::causal>;

struct room : public ByteRepresentable{

	using p = Handle<Level::strong, HandleAccess::all, room>;
	MemberList::p members;
	using posts_t = remote_set<Level::causal, post::p>::p;
	posts_t posts;

	room(newObject_f<MemberList::p> mf, newObject_f<decltype(posts)> pf)
		:members(MemberList::mke(mf)),
		 posts(remote_set<Level::causal, post::p>::mke(pf)) {}
	
	room(const decltype(members) &m, const decltype(posts) &p)
		:members(m),posts(p){}
	
	default_build
	DEFAULT_SERIALIZATION_SUPPORT(room,members,posts)
	
		void add_post(std::unique_ptr<VMObjectLog> &log, tracker::Tracker& trk, post::p pst){
		struct params {
			posts_t &posts; post::p &pst; MemberList::p &members;
		};
		params env_param{posts,pst,members};
		TRANSACTION(log,trk,env_param,
			do_op(Insert,env_params.posts,env_params.pst),
			let(hd) = env_params.members IN ( 
				WHILE (isValid(hd)) DO (
					let_remote(tmp) = hd IN (
						let_remote(tmp2) = $(tmp,val) IN (
							do_op(Insert,$(tmp2,inbox),env_params.pst)),
						hd = $(tmp,next)
						)
					)
				)
			); //*/
	}
	
	static void add_member(std::unique_ptr<VMObjectLog> &log, tracker::Tracker& trk, room::p room, user::p usr){
		struct params_t {room::p room; user::p usr;};
		params_t params{room,usr};
		TRANSACTION(log,trk,params,
			let_remote(rm) = params.room IN (
				$(rm,members) = $bld(MemberList, $$(params.usr), $$(membrs))
				)
			);
	}
};

struct groups {
	using p = typename remote_set<Level::causal, room::p>::p;
	default_build
};

struct TestArguments{
	unsigned long long start_time;
	std::size_t users_index;
	std::size_t rooms_index;
};

static_assert(std::is_pod<TestArguments>::value, "Error: need POD for serialization");

struct GroupRemember{
	std::vector<user> users;
	std::vector<room> rooms;
	Remember transaction_metadata;
	
};

using action_t = std::function<std::string (std::shared_ptr<GroupRemember>, int, TestArguments)>;

const std::vector<action_t>{{
		//post messagee
		[](std::shared_ptr<GroupRemember> mem, int tid, TestArguments args){
			auto log = mem->
				transaction_metadata.log_builder->
				template beginStruct<LoggedStructs::log>();
			mem->rooms.at(tid).add_post(log,mem->transaction_metadata.trk,"????");
		}
	}};

struct GroupTestParameters {
	using time_t = std::chrono::time_point<std::chrono::high_resolution_clock>;
	const time_t start_time{high_resolution_clock::now()};
	time_t last_rate_raise{start_time};
	Frequency current_rate{800_Hz};
	constexpr static Frequency increase_factor = 100_Hz;
	constexpr static seconds increase_delay = 15s;
	constexpr static minutes test_stop_time = 7min;
	constexpr static double percent_writes = .05;
	constexpr static double percent_strong = .7;
	
	pair<int,fake_time> choose_action() const {
		bool do_write = better_rand() < percent_writes;
		bool is_strong = (better_rand() < percent_strong || !causal_enabled) && strong_enabled;
		if (do_write && is_strong) return pair<int,fake_time>(0,micros(elapsed_time()));
		if (do_write && !is_strong) return pair<int,fake_time>(1,micros(elapsed_time()));
		if (!do_write && is_strong) return pair<int,fake_time>(2,micros(elapsed_time()));
		if (!do_write && !is_strong) return pair<int,fake_time>(3,micros(elapsed_time()));
		assert(false);
	}
	
	bool stop () const {
		return (high_resolution_clock::now() - start_time) >= test_stop_time;
	};

	milliseconds delay(){
		if (high_resolution_clock::now() - last_rate_raise > increase_delay){
			current_rate += increase_factor;
			last_rate_raise = high_resolution_clock::now();
		}
		return getArrivalInterval(current_rate);
	}
	
#define method_to_fun(foo,y...) [](auto& x){return x.foo(y);}
	std::string run_tests(PreparedTest<Remember,fake_time>& launcher){
		bool (*stop) (TestParameters&) = method_to_fun(stop);
		pair<int,fake_time> (*choose) (TestParameters&) = method_to_fun(choose_action);
		milliseconds (*delay) (TestParameters&) = method_to_fun(delay);
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
