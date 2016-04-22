#pragma once
#include <vector>
#include "Basics.hpp"

namespace{

	using namespace myria;
	using namespace myria::pgsql;

	namespace cmds {
		
		//hybrid section (same command works on both strong and causal)

		template<typename T>
		auto select_data(Level, T &trans, Table t, Name id){
			//std::cerr << "in select_data" << std::endl;
			//AtScopeEnd ase{[](){//std::cerr << "out" << std::endl;}};
			//discard(ase);
			static const std::string bs =
				"select data from \"BlobStore\" where index = 0 and ID = $1";
			static const std::string is = "select data from \"IntStore\" where index = 0 and ID = $1";
			switch(t) {
			case Table::BlobStore : return trans.prepared("select1",bs,id); 
			case Table::IntStore : return trans.prepared("select2",is,id); 
			}
			assert(false && "forgot a case");
		}

		template<typename T>
		auto obj_exists(Level, T &trans, Name id){
			//std::cerr << "in obj_exists" << std::endl;
			const static std::string query =
				"select ID from \"BlobStore\" where id = $1 union select id from \"IntStore\" where id = $1 limit 1";
			auto r = trans.prepared("exists",query,id);
			//std::cerr << "out" << std::endl;
			return r;
		}


		template<typename T>
		void remove(Level l, T &trans, Table, Name id){
			//std::cerr << "in remove" << std::endl;
			const static std::string bs =
				"delete from \"BlobStore\" where ID = $1";
			const static std::string is =
				"delete from \"IntStore\" where ID = $1";
			trans.prepared("Del1",bs,id);
			trans.prepared("Del2",is,id);
			//std::cerr << "out" << std::endl;
		}

		template<typename T>
		auto check_size(Level, T &trans, Table t, Name id){
			//std::cerr << "in check_size" << std::endl;
			assert(t == Table::BlobStore &&
				   "Error: size check non-sensical for non-blob data");
			const static std::string bs =
				"select max(octet_length(data)) from \"BlobStore\" where id = $1";
			auto r =  trans.prepared("Size",bs,id);
			//std::cerr << "out" << std::endl;
			return r;
		}

		//strong section

		template<typename T>
		void select_version_(T &trans, Table t, Name id, int& vers){
			static const std::string bs =
				"select Version from \"BlobStore\" where ID=$1";
			static const std::string is =
				"select Version from \"IntStore\" where ID=$1";
			auto &s = (t == Table::BlobStore ? bs : is );
			vers = -1;
			auto r = trans.prepared(s,s,id);
			assert(!r.empty());
			auto res = r[0][0].to(vers);
			assert(res);
			assert(vers != -1);
		}

		template<typename T, typename Blob>
		void update_data_s(T& trans, Table t, Name id, const Blob& b){
			static const std::string bs =
				"update \"BlobStore\" set data=$2,Version=Version + 1 where ID=$1";
			static const std::string is =
				"update \"IntStore\" set data=$2,Version=Version + 1 where ID=$1";
			switch(t) {
			case Table::BlobStore : trans.prepared("Updates1",bs,id,b); return;
			case Table::IntStore : trans.prepared("Updates2",is,id,b); return;
			}
			assert(false && "forgot a case");
		}

		template<typename T>
		void increment_s(T &trans, Table t, Name id){
			assert(t == Table::IntStore
				   && "Error: increment currently only defined on integers");
			const static std::string s =
				"update \"IntStore\" set data = data + 1 where id = $1";
			trans.prepared("Increment",s,id);
		}

		template<typename T, typename Blob>
		void initialize_with_id_s(T &trans, Table t, Name id, const Blob& b){
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
			constexpr int sizes = NUM_CAUSAL_GROUPS / NUM_CAUSAL_MASTERS;
			constexpr int overflow = NUM_CAUSAL_GROUPS - sizes * NUM_CAUSAL_MASTERS;
			return (k < overflow ? 0
					: (k - overflow - 1) / (NUM_CAUSAL_MASTERS)) + 1;
		}
		
		int md(int k){return (k % NUM_CAUSAL_GROUPS == 0 ? NUM_CAUSAL_GROUPS : k % NUM_CAUSAL_GROUPS);}

#define update_data_c_cmd(xx,set)	using namespace std;				\
		const static auto update_cmds = [&](){							\
			vector<pair<string,string> > ret;							\
			for (int _t = 0; _t < Table_max; ++_t){						\
				string t = table_name((Table)_t);						\
				for (int _k = 1; _k < (NUM_CAUSAL_GROUPS+1); ++_k){		\
					auto k = to_string(_k);								\
					stringstream main;									\
					main << "update " << t << "set data = " << set;		\
					for (int i = 1; i < NUM_CAUSAL_GROUPS; ++i) main << ", vc" << md(_k+i) << "=$"<<i+1; \
					main << ", lw = " << k								\
						 << " where id = $1 and index = " << group_mapper(_k); \
					ret.push_back(pair<string,string>{(t + "Update") + k + xx,main.str()}); \
				}														\
			}															\
			return ret;													\
		}(); assert(ends[2] < 30); 

		template<typename T>
		void select_version_(T &trans, Table t, Name id, std::array<int,NUM_CAUSAL_GROUPS>& vers){
			using namespace std;
			static const vector<pair<string,string> > v = [&](){
				vector<pair<string,string> > v;
				for (int i = 0; i < Table_max; ++i){
					v.push_back(pair<string,string>{string("Sel1") + std::to_string(i),
								string("select vc1,vc2,vc3,vc4 from ") + table_name((Table)i)
								+ " where ID=$1 and index=0"});
				}
				return v;
			}();
			auto &s = v.at((int)t);
			auto r = trans.prepared(s.first,s.second,id);
			assert(!r.empty());
			auto res1 = r[0][0].to(vers[0]);
			assert(res1);
			auto res2 = r[0][1].to(vers[1]);
			assert(res2);
			auto res3 = r[0][2].to(vers[2]);
			assert(res3);
			auto res4 = r[0][3].to(vers[3]);
			assert(res4);
			assert(vers[2] < 30);
		}
		
		template<typename T, typename Blob>
		void update_data_c(T &trans, Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, const Blob &b){
			update_data_c_cmd("x","$5");
			auto &p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
			trans.prepared(p.first,p.second,id,ends[md(k+1)-1],ends[md(k+2)-1],ends[md(k+3)-1],b);
		}
		
		
		template<typename T>
		void increment_c(T &trans, Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends){
			update_data_c_cmd("y","data + 1");
			auto &p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
			trans.prepared(p.first,p.second,id,ends[md(k+1)-1],ends[md(k+2)-1],ends[md(k+3)-1]);
		}

		template<typename T, typename Blob>
		void initialize_with_id_c(T &trans, Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, const Blob& b){
			assert(k > 0);
			using namespace std;
			static constexpr int n = NUM_CAUSAL_GROUPS;
			static constexpr int r = NUM_CAUSAL_MASTERS;
			const static auto v = [&](){
				vector<array<pair<string,string>,3 > > ret;
				for (int _t = 0; _t < Table_max; ++_t){
					stringstream main1;
					stringstream main2;
					stringstream main3;
					string t = table_name((Table)_t);
					main1 << "insert into " << t << " (vc1) "
						  << "select zeros from thirty_zeros where not"
						  <<"((select count(*) from " << t << "where id=0)"
						  <<" > 30);";
					main2 << "update "<< t << "set id=$1 where index in (select index from " << t << " where id=0 limit " << r+1 <<  ");";
					main3 << "update " << t << " set index=index-(select min(index) from " << t << " where id=$1) where id=$1;";
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
			auto &commands = v.at(((int)t) * n + (k-1));
                        for (std::size_t i = 0; i < commands.size(); ++i){
				auto &p = commands[i];
				if (i == 1 || i == 2)
					trans.prepared(p.first,p.second,id);
				else trans.exec(p.second);
			}
			update_data_c_cmd("z","$5");
			auto &p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
			trans.prepared(p.first,p.second,id,ends[md(k+1) - 1],ends[md(k+2) - 1],ends[md(k+3) - 1],b);
		}

		//entry points

		template<typename T, typename Ret>
		void select_version(Level l, T &tran, Table t, Name id,Ret& r){
			//std::cerr << "in select_version" << std::endl;
			//AtScopeEnd ase{[](){//std::cerr << "out" << std::endl;}};
			//discard(ase);
			if (l == Level::strong) {
				assert(std::is_scalar<Ret>::value);
			}
			return select_version_(tran,t,id,r);
		}
		

		template<typename T, typename Blob>
		void update_data(Level l, T &tran, Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, const Blob& b){
			//std::cerr << "in update_data" << std::endl;
			if (l == Level::strong) update_data_s(tran,t,id,b);
			else if (l == Level::causal) update_data_c(tran,t,k,id,ends,b);
			//std::cerr << "out" << std::endl;
		}


		template<typename T, typename Blob>
		void initialize_with_id(Level l, T &tran, Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, const Blob& b){
			//std::cerr << "in initialize_with_id" << std::endl;
			if (l == Level::strong) initialize_with_id_s(tran,t,id,b);
			else initialize_with_id_c(tran,t,k,id,ends,b);
			//std::cerr << "out" << std::endl;
		}
		template<typename T, typename Blob>
		void initialize_with_id(Level l, T &tran, Table t, Name id, const Blob& b){
			//std::cerr << "in initialize_with_id" << std::endl;
			if (l == Level::strong) initialize_with_id_s(tran,t,id,b);
			else assert(false && "not enough parameters for causal initialization");
			//std::cerr << "out" << std::endl;
		}

		template<typename T>
		void increment(Level l, T& tran, Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends){
			//std::cerr << "in increment" << std::endl;
			assert(t == Table::IntStore
				   && "Error: increment currently only defined on integers");
			if (l == Level::strong) increment_s(tran,t,id);
			else increment_c(tran,t,k,id,ends);
			//std::cerr << "out" << std::endl;
		}


	}

}
