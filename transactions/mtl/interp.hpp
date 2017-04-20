#pragma once
#include "AST_split.hpp"
#include "split_printer.hpp"
#include "environments.hpp"
#include "runnable_transaction.hpp"
#include "remote_interp.hpp"
#include "run_phase.hpp"

namespace myria {
namespace mtl {
namespace runnable_transaction {

	//remote interp
	template <typename store,typename phase1, typename connection_pack, typename ClientTracker>
	auto dispatch_to_runner(std::true_type*, mutils::DeserializationManager* dsm, ClientTracker& trk, connection_pack c, transaction<phase1>*, store& s, std::enable_if_t<phase1::label::run_remotely::value>* = nullptr)
	{
		return remote_interp<phase1>(dsm,trk,*c[0], s);
	}
	
	template <typename store, typename phase1, typename phase2, typename connection_pack, typename ClientTracker, typename... phase>
	auto dispatch_to_runner(std::true_type* choice, mutils::DeserializationManager* dsm, ClientTracker& trk, connection_pack c, transaction<phase1, phase2, phase...>*, store& s, std::enable_if_t<phase1::label::run_remotely::value>* = nullptr)
	{
		constexpr transaction<phase2, phase...>* remains{ nullptr };
		remote_interp<phase1>(dsm,trk,*c[0], s);
		return dispatch_to_runner(choice, dsm,trk,c.rest, remains, s);
	}

	//local interp
	template <typename store,typename phase1, typename connection_pack, typename choice, typename ClientTracker>
	auto dispatch_to_runner(choice*, mutils::DeserializationManager*, ClientTracker& trk, connection_pack, transaction<phase1>*, store& s,
													std::enable_if_t<(!phase1::label::run_remotely::value)
													|| (!choice::value)
													>* = nullptr)
	{
	  return common_interp<phase1>(s,trk.local_tracker);
	}
	
	template <typename store, typename phase1, typename phase2, typename connection_pack, typename Choice, typename ClientTracker, typename... phase>
	auto dispatch_to_runner(Choice* choice, mutils::DeserializationManager* dsm, ClientTracker& trk, connection_pack c, transaction<phase1, phase2, phase...>*, store& s,
													std::enable_if_t<(!phase1::label::run_remotely::value)
													|| (!Choice::value)
													>* = nullptr)
	{
		constexpr transaction<phase2, phase...>* remains{ nullptr };
		common_interp<phase1>(s,trk.local_tracker);
		return dispatch_to_runner(choice, dsm,trk,c, remains, s);
	}
	
  template <typename run_remotely, typename, typename split, typename store, typename connection_pack, typename ClientTracker>
	auto interp3(mutils::DeserializationManager* dsm, ClientTracker& trk, connection_pack c, void*, split* np, store& s){
		constexpr run_remotely* choice{nullptr};
		dispatch_to_runner(choice,dsm,trk,c, np, s);
	}

	template <typename run_remotely, typename ptr, typename split, typename store, typename connection_pack, typename ClientTracker>
	auto interp3(mutils::DeserializationManager* dsm, ClientTracker& trk, connection_pack c, ptr*, split* np, store& s, std::enable_if_t<!std::is_void<ptr>::value >* = nullptr){
		constexpr run_remotely* choice{nullptr};
		auto ret = dispatch_to_runner(choice,dsm,trk,c, np, s);
		return ret;
	}
	
	template <typename split, typename connection_pack, typename run_remotely, typename ClientTracker, typename... required>
  auto begin_interp2(mutils::DeserializationManager* dsm, ClientTracker& trk, connection_pack c, required... vals)
	{
		static_assert(std::is_same<run_remotely, std::true_type>::value || std::is_same<run_remotely, std::false_type>::value);
		constexpr split* np{ nullptr };
		using store_t = typename split::template all_store<required...>;
		// required should be struct value<>
		static_assert(is_store<store_t>::value);
		store_t store{vals... };
		constexpr run_remotely* run_remotely_v{nullptr};
		using ret_t = DECT(dispatch_to_runner(run_remotely_v,dsm,trk,c, np, store));
		constexpr ret_t *rt{nullptr};
		return interp3<run_remotely, ret_t>(dsm,trk,c, rt, np, store);
	}

  template <typename previous_transaction_phases, typename split, typename connection_pack, typename run_remotely, typename ClientTracker, typename... required>
  auto begin_interp(mutils::DeserializationManager* dsm, ClientTracker& trk, connection_pack c, required... vals)
	{
		using with_tracking = typename ClientTracker::template alternative_tracked_txn<previous_transaction_phases>;
		static_assert(std::is_same<with_tracking,with_tracking>::value);
		using without_tracking = split;
		if (trk.must_track()){
			//return begin_interp2<with_tracking,connection_pack,run_remotely,ClientTracker,required...>(dsm,trk,c,vals...);
			assert(false);
		}
		else {
			return begin_interp2<without_tracking,connection_pack,run_remotely,ClientTracker,required...>(dsm,trk,c,vals...);
		}
	}
}
}
}
