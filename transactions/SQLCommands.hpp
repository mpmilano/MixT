#pragma once

namespace{

	namespace cmds {
		auto select_max_id(Table t){
			if (t == Table::BlobStore)
				return "select max(ID) from \"BlobStore\"";
			else if (t == Table::IntStore)
				return "select max(ID) from \"IntStore\"";
			else assert(false && "forgot a case");
		}
		
		const std::string& select_version(Table t){
			static const std::string bs =
				"select Version from \"BlobStore\" where ID=";
			static const std::string is =
				"select Version from \"IntStore\" where ID=";
			switch(t) {
			case Table::BlobStore : return bs;
			case Table::IntStore : return is;
			}
			assert(false && "forgot a case");
		}
		
		const std::string& select_data(Table t){
			static const std::string bs =
				"select data from \"BlobStore\" where ID = ";
			static const std::string is = "select data from \"IntStore\" where ID = ";
			switch(t) {
			case Table::BlobStore : return bs;
			case Table::IntStore : return is;
			}
			assert(false && "forgot a case");
		}
		
		const std::string& update_data(Table t){
			static const std::string bs =
				"update \"BlobStore\" set data=$1,Version=$2 where ID=";
			static const std::string is =
					"update \"IntStore\" set data=$1,Version=$2 where ID=";
				switch(t) {
				case Table::BlobStore : return bs;
				case Table::IntStore : return is;
				}
			assert(false && "forgot a case");
		}

		const auto& obj_exists(){
			const static std::string query =
				"select id from \"BlobStore\" where id = $1 union select id from \"IntStore\" where id = $1 limit 1";
			return query;
		}

		const auto& initialize_with_id(Table t){
			const static std::string bs =
				"INSERT INTO \"BlobStore\" (id,data) VALUES ($1,$2)";
			const static std::string is =
				"INSERT INTO \"IntStore\" (id,data) VALUES ($1,$2)";
			switch(t) {
			case Table::BlobStore : return bs;
			case Table::IntStore : return is;
			}
			assert(false && "forgot a case");			
		}

		const auto& remove(Table t){
			const static std::string bs =
				"delete from \"BlobStore\" where ID = ";
			const static std::string is =
				"delete from \"IntStore\" where ID = ";
			switch(t) {
			case Table::BlobStore : return bs;
			case Table::IntStore : return is;
			}
			assert(false && "forgot a case");
		}

		const auto& check_size(Table t){
			assert(t == Table::BlobStore &&
				   "Error: size check non-sensical for non-blob data");
			const static std::string bs =
				"select octet_length(data) from \"BlobStore\" where id = $1";
			return bs;
		}

		const auto& increment(Table t){
			assert(t == Table::IntStore
				   && "Error: increment currently only defined on integers");
			const static std::string s =
				"update \"IntStore\" set data = data + 1 where id = $1";
			return s;
		}
	}
}
