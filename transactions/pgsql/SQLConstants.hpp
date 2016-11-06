#pragma once
#include "mutils.hpp"
#include "test_utils.hpp"
#include "cexprutils.hpp"
#include "simple_rpc.hpp"

namespace myria {
	namespace pgsql {

		namespace conn_space = mutils::simple_rpc;
		
		constexpr const int strong_sql_port = 9876;
		constexpr const int causal_sql_port = 9877;

		enum class Table{
			BlobStore = 0,IntStore = 1
				};
		static constexpr int Table_max = 2;;

		static const constexpr int strong_ip_addr{static_cast<int>(mutils::get_strong_ip())};
		static const constexpr int causal_ip_addr{static_cast<int>(mutils::get_causal_ip())};

		static const constexpr int repl_group{CAUSAL_GROUP};

		constexpr mutils::CTString table_name(Table t){
			using namespace mutils;
			constexpr auto bs = "\"BlobStore\"";
			constexpr auto is = "\"IntStore\"";
			switch (t){
			case Table::BlobStore : return CTString{} + bs;
			case Table::IntStore : return CTString{} + is;
			};
		}		
	}
}
