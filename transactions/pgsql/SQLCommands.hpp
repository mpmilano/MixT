#pragma once
#include <vector>
#include "Basics.hpp"

namespace{

	using namespace myria;
	using namespace myria::pgsql;

	using namespace mutils;

	namespace cmds {
		
		//hybrid section (same command works on both strong and causal)



		template<typename T>
		auto obj_exists(Level, T &trans, Name id){
			//std::cerr << "in obj_exists" << std::endl;
			const static std::string query =
				"select ID from \"BlobStore\" where id = $1 union select id from \"IntStore\" where id = $1 limit 1";
			auto r = trans.prepared(TransactionNames::exists,query,id);
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
			trans.prepared(TransactionNames::Del1,bs,id);
			trans.prepared(TransactionNames::Del2,is,id);
			//std::cerr << "out" << std::endl;
		}

		//strong section

		template<typename T>
		auto select_version_s(T &trans, Table t, Name id){
			static const std::string bs =
				"select Version from \"BlobStore\" where ID=$1";
			static const std::string is =
				"select Version from \"IntStore\" where ID=$1";
			auto &s = (t == Table::BlobStore ? bs : is );
			return trans.prepared((t == Table::BlobStore ?
								   TransactionNames::select_version_s_b :
								   TransactionNames::select_version_s_i
									  ),s,id);
		}
		
		template<typename T>
		auto select_version_data_size_s(Level l, T &trans, Table t, Name id){
			assert(l == Level::strong);
			//std::cerr << "in select_data" << std::endl;
			//AtScopeEnd ase{[](){//std::cerr << "out" << std::endl;}};
			//discard(ase);
			static const std::string bs =
				"select version, data, max(octet_length(data)) as size from \"BlobStore\" where index = 0 and ID = $1 group by version,data";
			static const std::string is = "select version, data from \"IntStore\" where index = 0 and ID = $1";
			switch(t) {
			case Table::BlobStore : return trans.prepared(TransactionNames::select1,bs,id); 
			case Table::IntStore : return trans.prepared(TransactionNames::select2,is,id); 
			}
			assert(false && "forgot a case");
		}

		template<typename T, typename Blob>
		auto update_data_s(T& trans, Table t, Name id, const Blob& b){
			static const std::string bs =
				"update \"BlobStore\" set data=$2,Version=Version + 1 where ID=$1 returning version";
			static const std::string is =
				"update \"IntStore\" set data=$2,Version=Version + 1 where ID=$1 returning version";
			switch(t) {
			case Table::BlobStore : return trans.prepared(TransactionNames::Updates1,bs,id,b);
			case Table::IntStore : return trans.prepared(TransactionNames::Updates2,is,id,b);
			}
			assert(false && "forgot a case");
		}

		template<typename T>
		auto increment_s(T &trans, Table t, Name id){
			assert(t == Table::IntStore
				   && "Error: increment currently only defined on integers");
			const static std::string s =
				"update \"IntStore\" set data = data + 1 where id = $1 returning version";
			return trans.prepared(TransactionNames::Increment,s,id);
		}

		template<typename T, typename Blob>
		void initialize_with_id_s(T &trans, Table t, Name id, const Blob& b){
			const static std::string bs =
				"INSERT INTO \"BlobStore\" (id,data) VALUES ($1,$2)";
			const static std::string is =
				"INSERT INTO \"IntStore\" (id,data) VALUES ($1,$2)";
			switch(t) {
			case Table::BlobStore : trans.prepared(TransactionNames::Insert1,bs,id,b); return;
			case Table::IntStore : trans.prepared(TransactionNames::Insert2,is,id,b); return;
			}
			assert(false && "forgot a case");
			
		}


		//causal section 

		constexpr int group_mapper(int k){
			if (k < 1) assert(false && "Error: k is 1-indexed");
			constexpr int sizes = NUM_CAUSAL_GROUPS / NUM_CAUSAL_MASTERS;
			constexpr int overflow = NUM_CAUSAL_GROUPS - sizes * NUM_CAUSAL_MASTERS;
			return (k < overflow ? 0
					: (k - overflow - 1) / (NUM_CAUSAL_MASTERS)) + 1;
		}
		
		constexpr int md(int k){
			return (k % NUM_CAUSAL_GROUPS == 0 ? NUM_CAUSAL_GROUPS : k % NUM_CAUSAL_GROUPS);
		}

		constexpr auto update_data_c_cmd(TransactionNames const (&xx)[Table_max * NUM_CAUSAL_GROUPS],
										 char const * const set) {
			using namespace std;
			struct ReturnThis{
				static constexpr CTString vcs1(unsigned int i, unsigned int stop, unsigned int _k){
					return CTString{} + ", vc" + md(_k+i)+ "=$"+ (i+1) +
										  ( i==(stop-1) ? CTString{} : vcs1(i+1,stop,_k));
				}

				static constexpr CTString vcs2(unsigned int i, unsigned int stop){
					return CTString{} + "vc" + i + "," + (i == (stop-1)? CTString{} : vcs2(i+1,stop));
				}
				
				pair<TransactionNames,CTString> ret[Table_max * (NUM_CAUSAL_GROUPS+1)]
					{pair<TransactionNames,CTString>{TransactionNames::MAX,CTString{} }};

				pair<TransactionNames,string> at(std::size_t s) const {
					return make_pair(ret[s].first,string{ret[s].second.str});
				}
			};

			static_assert(NUM_CAUSAL_GROUPS == 4,"This is assumed");
			static_assert(Table_max == 2,"This is assumed");

			ReturnThis ret;
			int index{0};
			for (int _t = 0; _t < Table_max; ++_t){
				CTString t {table_name((Table)_t)};
				for (int _k = 1; _k < (NUM_CAUSAL_GROUPS+1); ++_k){
					auto k = to_ctstring(_k);
					CTString main = CTString{}
					+ "update " + t + " set data = " + set
						+ ReturnThis::vcs1(1,NUM_CAUSAL_GROUPS,_k)
						+ ", lw = " + k
						+ " where id = $1 and index = " + group_mapper(_k)
						+ " returning " + ReturnThis::vcs2(1,NUM_CAUSAL_GROUPS)
						+ "vc" + NUM_CAUSAL_GROUPS;
					ret.ret[index].first = xx[_k + NUM_CAUSAL_GROUPS*_t -1 ];
					ret.ret[index].second = main;
					++index;
				}
			}
			return ret;
		}

		template<typename T>
		auto select_version_c(T &trans, Table t, Name id){
			using namespace std;
			static const vector<pair<TransactionNames,string> > v = [&](){
				vector<pair<TransactionNames,string> > v;
				for (int i = 0; i < Table_max; ++i){
					v.push_back(pair<TransactionNames,string>{
							((Table)i == Table::IntStore ?
							 TransactionNames::Sel1i : TransactionNames::Sel1b),
								string("select vc1,vc2,vc3,vc4 from ") + table_name((Table)i).str
								+ " where ID=$1 and index=0"});
				}
				return v;
			}();
			auto &s = v.at((int)t);
			return trans.prepared(s.first,s.second,id);
		}

		template<typename T>
		auto select_version_data_size_c(Level l, T &trans, Table t, Name id){
			assert(l == Level::causal);
			//std::cerr << "in select_data" << std::endl;
			//AtScopeEnd ase{[](){//std::cerr << "out" << std::endl;}};
			//discard(ase);
			static const std::string bs =
				"select vc1,vc2,vc3,vc4, data, max(octet_length(data)) as size from \"BlobStore\" where index = 0 and ID = $1 group by vc1,vc2,vc3,vc4,data ";
			static const std::string is = "select vc1,vc2,vc3,vc4, data from \"IntStore\" where index = 0 and ID = $1";
			switch(t) {
			case Table::BlobStore : return trans.prepared(TransactionNames::select1,bs,id); 
			case Table::IntStore : return trans.prepared(TransactionNames::select2,is,id); 
			}
			assert(false && "forgot a case");
		}
		
		template<typename T, typename Blob>
		auto update_data_c(T &trans, Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, const Blob &b){
			static const constexpr TransactionNames tnames[NUM_CAUSAL_GROUPS * Table_max] = {
				TransactionNames::udc1,TransactionNames::udc4,TransactionNames::udc7,
				TransactionNames::udc2,TransactionNames::udc5,TransactionNames::udc8,
				TransactionNames::udc3,TransactionNames::udc6,
			};
			static const constexpr auto update_cmds = update_data_c_cmd(tnames,"$5");
			auto p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
			return trans.prepared(p.first,p.second,id,ends[md(k+1)-1],ends[md(k+2)-1],ends[md(k+3)-1],b);
		}
		
		
		template<typename T>
		auto increment_c(T &trans, Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends){
			static const constexpr TransactionNames tnames[NUM_CAUSAL_GROUPS * Table_max] = {
				TransactionNames::ic1,TransactionNames::ic4,TransactionNames::ic7,
				TransactionNames::ic2,TransactionNames::ic5,TransactionNames::ic8,
				TransactionNames::ic3,TransactionNames::ic6,
			};
			static const constexpr auto update_cmds = update_data_c_cmd(tnames,"data + 1");
			auto p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
			return trans.prepared(p.first,p.second,id,ends[md(k+1)-1],ends[md(k+2)-1],ends[md(k+3)-1]);
		}

		template<typename T, typename Blob>
		void initialize_with_id_c(T &trans, Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, const Blob& b){
			assert(k > 0);


			const static std::string bs = "INSERT INTO \"BlobStore\" (index,id) VALUES (0,$1), (1,$1), (2,$1)";
			const static std::string is = "INSERT INTO \"IntStore\" (index,id) VALUES (0,$1), (1,$1), (2,$1)";
			if (t == Table::IntStore){
				trans.prepared(TransactionNames::initci,is,id);
			} else trans.prepared(TransactionNames::initcb,bs,id);
			
			static const constexpr TransactionNames tnames[NUM_CAUSAL_GROUPS * Table_max] = {
				TransactionNames::udc1,TransactionNames::udc4,TransactionNames::udc7,
				TransactionNames::udc2,TransactionNames::udc5,TransactionNames::udc8,
				TransactionNames::udc3,TransactionNames::udc6,
			};
			constexpr const auto update_cmds = update_data_c_cmd(tnames,"$5");
			const auto &p = update_cmds.at(((int)t) * (NUM_CAUSAL_GROUPS) + (k-1));
			trans.prepared(p.first,p.second,id,ends[md(k+1) - 1],ends[md(k+2) - 1],ends[md(k+3) - 1],b);
		}

		//entry points

		template<typename T>
		auto select_version(Level l, T &tran, Table t, Name id){
			//std::cerr << "in select_version" << std::endl;
			//AtScopeEnd ase{[](){//std::cerr << "out" << std::endl;}};
			//discard(ase);
			if (l == Level::strong) {
				return select_version_s(tran,t,id);
			}
			else return select_version_c(tran,t,id);
		}

		
		template<typename T>
		auto select_version_data_size(Level l, T &trans, Table t, Name id){
			return (l == Level::strong ?
					select_version_data_size_s(l,trans,t,id) :
					select_version_data_size_c(l,trans,t,id));
		}

		template<typename T, typename Blob>
		auto update_data(Level l, T &tran, Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends, const Blob& b){
			//std::cerr << "in update_data" << std::endl;
			if (l == Level::strong) return update_data_s(tran,t,id,b);
			else if (l == Level::causal) return update_data_c(tran,t,k,id,ends,b);
			else assert(false && "there is a third case now?");
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
			assert(false && "not enough parameters for causal initialization");
			//std::cerr << "out" << std::endl;
		}

		template<typename T>
		auto increment(Level l, T& tran, Table t, int k, Name id, const std::array<int,NUM_CAUSAL_GROUPS> &ends){
			//std::cerr << "in increment" << std::endl;
			assert(t == Table::IntStore
				   && "Error: increment currently only defined on integers");
			if (l == Level::strong) return increment_s(tran,t,id);
			else return increment_c(tran,t,k,id,ends);
			//std::cerr << "out" << std::endl;
		}


	}

}
