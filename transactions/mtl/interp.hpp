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
	template <typename store,typename phase1, typename connection_pack>
	auto dispatch_to_runner(mutils::DeserializationManager* dsm, connection_pack c, transaction<phase1>*, store& s, std::enable_if_t<phase1::label::run_remotely::value>* = nullptr)
	{
		return remote_interp<phase1>(dsm,*c[0], s);
	}
	
	template <typename store, typename phase1, typename phase2, typename connection_pack, typename... phase>
	auto dispatch_to_runner(mutils::DeserializationManager* dsm, connection_pack c, transaction<phase1, phase2, phase...>*, store& s, std::enable_if_t<phase1::label::run_remotely::value>* = nullptr)
	{
		constexpr transaction<phase2, phase...>* remains{ nullptr };
		remote_interp<phase1>(dsm,*c[0], s);
		return dispatch_to_runner(dsm,c.rest, remains, s);
	}

	//local interp
	template <typename store,typename phase1, typename connection_pack>
	auto dispatch_to_runner(mutils::DeserializationManager*, connection_pack, transaction<phase1>*, store& s, std::enable_if_t<!phase1::label::run_remotely::value>* = nullptr)
	{
		return common_interp<phase1>(s);
	}
	
	template <typename store, typename phase1, typename phase2, typename connection_pack, typename... phase>
	auto dispatch_to_runner(mutils::DeserializationManager* dsm, connection_pack c, transaction<phase1, phase2, phase...>*, store& s, std::enable_if_t<!phase1::label::run_remotely::value>* = nullptr)
	{
		constexpr transaction<phase2, phase...>* remains{ nullptr };
		common_interp<phase1>(s);
		return dispatch_to_runner(dsm,c, remains, s);
	}
	
	template <typename, typename split, typename store, typename connection_pack>
	auto interp3(mutils::DeserializationManager* dsm, connection_pack c, void*, split* np, store& s){
		dispatch_to_runner(dsm,c, np, s);
	}

	template <typename ptr, typename split, typename store, typename connection_pack>
	auto interp3(mutils::DeserializationManager* dsm, connection_pack c, ptr*, split* np, store& s, std::enable_if_t<!std::is_void<ptr>::value >* = nullptr){
		auto ret = dispatch_to_runner(dsm,c, np, s);
		return ret;
	}

	template <typename split, typename connection_pack, typename... required>
	auto begin_interp(mutils::DeserializationManager* dsm, connection_pack c, required... vals)
	{
		constexpr split* np{ nullptr };
		using store_t = typename split::template all_store<required...>;
		// required should be struct value<>
		static_assert(is_store<store_t>::value);
		store_t store{initialize_store_values{}, vals... };
		using ret_t = DECT(dispatch_to_runner(dsm,c, np, store));
		constexpr ret_t *rt{nullptr};
		return interp3<ret_t>(dsm,c, rt, np, store);
}
}
}
}
