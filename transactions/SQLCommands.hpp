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

		template<typename T>
		const auto& obj_exists(Level, T &trans, int id){
			const static std::string query =
				"select id from \"BlobStore\" where id = $1 union select id from \"IntStore\" where id = $1 limit 1";
			return trans.prepared("exists",query,id);
		}


		template<typename T>
		void remove(Level l, T &trans, Table t, int id){
			const static std::string bs =
				"delete from \"BlobStore\" where ID = ";
			const static std::string is =
				"delete from \"IntStore\" where ID = ";
			switch(t) {
			case Table::BlobStore : trans.prepared("Del",bs,id); return;
			case Table::IntStore : trans.prepared("Del",is,id); return;
			}
			assert(false && "forgot a case");
		}

		template<typename T>
		const auto& check_size(Level, T &trans, Table t, int id){
			assert(t == Table::BlobStore &&
				   "Error: size check non-sensical for non-blob data");
			const static std::string bs =
				"select max(octet_length(data)) from \"BlobStore\" where id = $1";
			return trans.prepared("Size",bs,id);
		}

		//strong section

		template<typename T>
		const auto& increment_s(T &trans, Table t, int id){
			assert(t == Table::IntStore
				   && "Error: increment currently only defined on integers");
			const static std::string s =
				"update \"IntStore\" set data = data + 1 where id = $1";
			trans.prepared("Increment",s,id);
		}

		template<typename T, typename Blob>
		void initialize_with_id_s(T &trans, Table t, int id, const Blob& b){
			const static std::string bs =
				"INSERT INTO \"BlobStore\" (id,data) VALUES ($1,$2)";
			const static std::string is =
				"INSERT INTO \"IntStore\" (id,data) VALUES ($1,$2)";
			switch(t) {
			case Table::BlobStore : trans.prepared("Insert1",bs,id,b); return;
			case Table::IntStore : trans.prepared("Insert2",is,id,b); return;
			}
			assert(false && "forgot a case");			
		}


		//causal section 

		static constexpr int group_mapper(int k){
			if (k < 1) assert(false && "Error: k is 1-indexed");
			static constexpr int sizes = NUM_CAUSAL_GROUPS / NUM_CAUSAL_MASTERS;
			static constexpr int overflow = NUM_CAUSAL_GROUPS - sizes * NUM_CAUSAL_MASTERS;
			return (k < overflow ? 0
					: (k - overflow - 1) / (NUM_CAUSAL_MASTERS)) + 1;
		}
		
		int md4(int k){return (k % 4 == 0 ? 4 : k % 4);}

		const auto& update_data_c_cmd(char const * const set, Table t, int k){
			const static auto update_cmds = [&](){
				vector<pair<string,string> > > ret;
				for (int _t = 0; _t < Table_max; ++_t){
					string t = table_name((Table)_t);
					for (int _k = 1; _k < (NUM_CAUSAL_GROUPS+1); ++_k){
						auto k = to_string(_k);
						stringstream main;
						main << "update " << t << "set data = " << set
							 << ", vc" << md4(k+1) << "=$2, vc"<<md4(k+2)<<" = $3, vc"<<md4(k+3)<<" = $4, lw = " << k
							 << " where \"ID\" = $1 and index = " << group_mapper(_k);
						ret.push_back(pair<string,string>{(t + "Update") + k,main.str()});
					}
				}
				return ret;
			}();
			return v.at(((int)t) * (NUM_CAUSAL_GROUPS) + k);
		}

		template<typename T>
			void increment_c(T &trans, Table t, int k, int id, const std::array<int,4> &ends){
			auto &p = update_data_c_cmd("data + 1",t,k);
			trans.prepared(p.first,p.second,id,ends[md4(k+1)],ends[md4(k+2)],ends[md4(k+3)]);
		}

		template<typename T, typename Blob>
			void initialize_with_id_c(T &trans, Table t, int k, int id, const std::array<int,4> &ends, const Blob& b){
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
					for (int _k = 1; _k < (n+1); ++_k){
						auto k = to_string(_k);
						array<pair<string,string>,3> arr;
						arr[0]= pair<string,string>{(t + "Create1") + k,main1.str()};
						arr[1]= pair<string,string>{(t + "Create2") + k,main2.str()};
						arr[2]= pair<string,string>{(t + "Create3") + k,main3.str()};
						ret.push_back(arr);
					}
				}
				return ret;
			}();
			auto &commands = v.at(((int)t) * n + k);
			for (int i = 0; i < commands.size(); ++i){
				if (i == 1 || i == 2)
					trans.prepared(p.first,p.second,id);
				else trans.prepared(p.first,p.second);
			}
			auto &p = update_data_c_cmd("$5",t,k);
			trans.prepared(p.first,p.second,id,ends[md4(k+1)],ends[md4(k+2)],ends[md4(k+3)],blob);
		}

		//entry points

		template<typename T, typename Blob>
			void initialize_with_id(Level l, T &tran, Table t, int k, int id, const std::array<int,4> &ends, const Blob& b){
			if (l == Level::strong) initialize_with_id_s(tran,id,b);
			else initialize_with_id_c(tran,t,k,id,ends,b);
		}
		template<typename T, typename Blob>
			void initialize_with_id(Level l, T &tran, Table t, int id, const Blob& b){
			if (l == Level::strong) initialize_with_id_s(tran,t,id,b);
			else assert(false && "not enough parameters for causal initialization");
		}

		template<typename T>
			void increment(Level l, T& tran, Table t, int k, int id, const std::array<int,4> &ends, int b){
			assert(t == Table::IntStore
				   && "Error: increment currently only defined on integers");
			if (l == Level::strong) increment_s(tran,t,id,b);
			else increment_c(tran,t,k,id,ends,b);
		}


	}
}
