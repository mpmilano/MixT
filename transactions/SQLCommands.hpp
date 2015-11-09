#pragma once

namespace{

	namespace cmds {
		
		const std::string& select_version_strong(Table t){
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

		const std::string& select_version_causal(Table t){
			static const std::string pre =
				"select version from ";
			static const std::string post = " where ID=";
			return pre + tname(t) + post;
		}
		
		const auto& select_version(Level l, Table t){
			if (l == Level::strong) return select_version_strong(t);
			else return select_version_causal(t);
		}
		
		const std::string& select_data(Level l, Table t){
			static const std::string bs =
				"select data from \"BlobStore\" where ID = ";
			static const std::string is = "select data from \"IntStore\" where ID = ";
			switch(t) {
			case Table::BlobStore : return bs;
			case Table::IntStore : return is;
			}
			assert(false && "forgot a case");
		}
		
		const std::string& update_data(Level l, Table t){
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

		const auto& obj_exists(Level l){
			const static std::string query =
				"select id from \"BlobStore\" where id = $1 union select id from \"IntStore\" where id = $1 limit 1";
			return query;
		}

		const auto& initialize_with_id_s(Level l, Table t){
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

		const auto& initialize_with_id_c(Table t, int n, int k){
			const static std::vector<std::string> v = [&](){
				std::vector<std::string> ret;
				for (int _t = 0; _t < Table_max; ++_t){
					std::stringstream _main;
					std::string t = table_name((Table)_t);
					main << "update "<< t << "set \"ID\"=$1 where index in (select index from " << t << " where \"ID\"=0 limit " << n+1 <<  ");"
						 << "update " << t << " set index=index-(select min(index) from " << t << " where \"ID\"=$1) where \"ID\"=$1;";
					const std::string main = _main.str();
					for (int _k = 0; _k < n; ++_k){
						auto k = std::to_string(_k);
						std::stringstream command;
						command
							<< main <<
							"update counters set counter = counter + 1 where index = " k <<"; "
							<<"update " << t <<" set data=$2 where \"ID\"=$1 and index = " << k << "or index = 0;"
							<<"update " << t << "set vc" << k << "=(select counter from counters where index = " << k << ") where index = " << k << " or index = 0;";
						ret.push_back(command.str());
					}
				}
				return ret;
			}();
			return ret.at(((int)t) * );
		}
		
		const auto& initialize_with_id(Level l){
			if (l == Level::strong) return initialize_with_id_s(t);
			else return initialize_with_id_c(t);
		}

		const auto& remove(Level l, Table t){
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

		const auto& check_size(Level l, Table t){
			assert(t == Table::BlobStore &&
				   "Error: size check non-sensical for non-blob data");
			const static std::string bs =
				"select octet_length(data) from \"BlobStore\" where id = $1";
			return bs;
		}

		const auto& increment(Level l, Table t){
			assert(t == Table::IntStore
				   && "Error: increment currently only defined on integers");
			const static std::string s =
				"update \"IntStore\" set data = data + 1 where id = $1";
			return s;
		}
	}
}
