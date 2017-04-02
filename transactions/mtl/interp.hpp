#pragma once
#include "AST_split.hpp"
#include "split_printer.hpp"
#include "environments.hpp"
#include "runnable_transaction.hpp"
#include "remote_interp.hpp"

namespace myria {
namespace mtl {
namespace runnable_transaction {
	
	template <typename store,typename phase1>
	auto interp2(DeserializationManager* dsm, mutils::connection& c, transaction<phase1>*, store& s)
	{
		return remote_interp<phase1>(dsm,c, s);
	}
	
	template <typename store, typename phase1, typename phase2, typename... phase>
	auto interp2(DeserializationManager* dsm, mutils::connection& c, transaction<phase1, phase2, phase...>*, store& s)
	{
		constexpr transaction<phase2, phase...>* remains{ nullptr };
		remote_interp<phase1>(dsm,c, s);
		return interp2(dsm,c, remains, s);
	}
	template <typename, typename split, typename store>
	auto interp3(DeserializationManager* dsm, mutils::connection& c, void*, split* np, store& s){
		interp2(dsm,c, np, s);
	}

	template <typename ptr, typename split, typename store>
	auto interp3(DeserializationManager* dsm, mutils::connection& c, ptr*, split* np, store& s, std::enable_if_t<!std::is_void<ptr>::value >* = nullptr){
		auto ret = interp2(dsm,c, np, s);
		return ret;
	}

	template <typename split, typename... required>
	auto begin_interp(DeserializationManager* dsm, mutils::connection& c, required... vals)
	{
		constexpr split* np{ nullptr };
		using store_t = typename split::template all_store<required...>;
		// required should be struct value<>
		static_assert(is_store<store_t>::value);
		store_t store{initialize_store_values{}, vals... };
		using ret_t = DECT(interp2(dsm,c, np, store));
		constexpr ret_t *rt{nullptr};
		return interp3<ret_t>(dsm,c, rt, np, store);
}
}
}
}
