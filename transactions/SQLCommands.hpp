#pragma once

namespace{

	namespace cmds {

		static constexpr int group_mapper(int k){
			if (k < 1) assert(false && "Error: k is 1-indexed");
			static constexpr int sizes = NUM_CAUSAL_GROUPS / NUM_CAUSAL_MASTERS;
			static constexpr int overflow = NUM_CAUSAL_GROUPS - sizes * NUM_CAUSAL_MASTERS;
			return (k < overflow ? 0
					: (k - overflow - 1) / (NUM_CAUSAL_MASTERS)) + 1;
		}
		
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

		template<typename T, typename Blob>
		void initialize_with_id(Level l, T &tran, Table t, int k, int id, const Blob& b){

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


		template<typename T, typename Blob>
		void initialize_with_id_c(T &trans, Table t, int k, int id, const Blob& b){
			using namespace std;
			static constexpr int n = NUM_CAUSAL_GROUPS;
			static constexpr int r = NUM_CAUSAL_MASTERS;
			const static auto v = [&](){
				vector<array<pair<string,string>,6 > > ret;
				for (int _t = 0; _t < Table_max; ++_t){
					stringstream main1;
					stringstream main2;
					stringstream main3;
					string t = table_name((Table)_t);
					main1 << "insert into " << t << " (vc1) "
						 << "select zeros from thirty_zeros where not"
						 <<"((select count(*) from " << t << "where \"ID\"=0)"
						 <<" > 30);";
					main2 << "update "<< t << "set \"ID\"=$1 where index in (select index from " << t << " where \"ID\"=0 limit " << r+1 <<  ");";
					main3 << "update " << t << " set index=index-(select min(index) from " << t << " where \"ID\"=$1) where \"ID\"=$1;";
					const string main = _main.str();
					for (int _k = 0; _k < n; ++_k){
						auto k = to_string(_k);
						stringstream main4;
						main4 << "update " << t << "set data = $2, lw = " << k << " where \"ID\" = $1 and index = " << group_mapper(k);
						array<pair<string,string>,4> arr;
						arr[0]= pair<string,string>{(t + "Create1") + k,main1.str()};
						arr[1]= pair<string,string>{(t + "Create2") + k,main2.str()};
						arr[2]= pair<string,string>{(t + "Create3") + k,main3.str()};
						arr[3]= pair<string,string>{(t + "Create4") + k,main4.str()};
						ret.push_back(arr);
					}
				}
				return ret;
			}();
			auto &commands = ret.at(((int)t) * n + k);
			for (int i = 0; i < commands.size(); ++i){
				if (i == 1 || i == 2)
					trans.prepared(p.first,p.second,id);
				else if (i == 4)
					trans.prepared(p.first,p.second,id,blob);
				else trans.prepared(p.first,p.second);
			}
		}

		template<typename T, typename Blob>
		void initialize_with_id(Level l, T &tran, Table t, int k, int id, const Blob& b){
			if (l == Level::strong) initialize_with_id_s(tran,t,k,id,b);
			else initialize_with_id_c(tran,t,k,id,b);
		}
	}
}
