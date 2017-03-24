#pragma once
#include "mutils.hpp"
#include "batched_connection.hpp"
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
				exists = 0,
					Del1 = 1, Del2 = 2,
					select_version_s_i = 3, select_version_s_b = 4,
					select1 = 5, select2 = 6,
					Updates1 = 7, Updates2 = 8,
					Increment = 9,
					Insert1 = 10, Insert2 = 11,
					Sel1i = 12, Sel1b = 13,
					udc1 = 14,udc2 = 15,udc3 = 16,udc4 = 17,udc5 = 18,udc6 = 19,udc7 = 20,udc8 = 21,
					ic1 = 22,ic2 = 23,ic3 = 24,ic4 = 25,ic5 = 26,ic6 = 27,ic7 = 28,ic8 = 29,initci = 30,initcb = 31,
					usertest1 = 32,usertest2 = 33,usertest3 = 34,usertest4 = 35,
					MAX = 36
					};
		}
	}
}
