#pragma once
#include "cexprutils.hpp"

namespace myria {
	namespace pgsql {
		constexpr const int strong_sql_port = 9876;
		constexpr const int causal_sql_port = 9877;

		enum class Table{
			BlobStore = 0,IntStore = 1
				};
		static constexpr int Table_max = 2;;

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
