#pragma once
#include "mutils.hpp"
#include "test_utils.hpp"
#include "cexprutils.hpp"
#include <iostream>

namespace myria {
	namespace pgsql {

		enum class Level {strong,causal};
		std::ostream& operator<<(std::ostream&, Level);

		namespace conn_space = mutils::batched_connection;
		
		constexpr const int strong_sql_port = 9876;
		constexpr const int causal_sql_port = 9877;

		enum class Table{
			BlobStore = 0,IntStore = 1
				};
		static constexpr int Table_max = 2;;

		static const constexpr int strong_ip_addr{static_cast<int>(mutils::get_strong_ip())};
		static const constexpr int causal_ip_addr{static_cast<int>(mutils::get_causal_ip())};

		static const constexpr int repl_group{CAUSAL_GROUP};

		constexpr mutils::CexprString table_name(Table t){
			using namespace mutils;
			constexpr auto bs = "\"BlobStore\"";
			constexpr auto is = "\"IntStore\"";
			switch (t){
			case Table::BlobStore : return CexprString{} + bs;
			case Table::IntStore : return CexprString{} + is;
			};
		}
		namespace local{
			enum class LocalTransactionNames{
				exists, Del1, Del2, select_version_s_i, select_version_s_b,
					select1, select2, Updates1, Updates2, Increment, Insert1, Insert2,
					Sel1i,Sel1b,udc1,udc2,udc3,udc4,udc5,udc6,udc7,udc8,
					ic1,ic2,ic3,ic4,ic5,ic6,ic7,ic8,initci,initcb,
					MAX
					};
		}
	}
}
