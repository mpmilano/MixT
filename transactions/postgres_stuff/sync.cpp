#include <pqxx/pqxx>
#include <array>
#include <cassert>
#include <iostream>

using namespace pqxx;
using namespace std;

int main(){
	const string id = to_string(
#include "../big_prime"
		);
	try { 
	pqxx::connection conn_strong;
	conn_strong.prepare("Update",
						string("update \"BlobStore\" set data=$1 where id= ") + id);
	pqxx::connection conn_causal;
	conn_causal.prepare("selectit","select max(vc1) as vc1,max(vc2) as vc2,max(vc3) as vc3,max(vc4) as vc4 from (select max(vc1) as vc1,max(vc2) as vc2,max(vc3) as vc3,max(vc4) as vc4 from \"BlobStore\" union select max(vc1) as vc1,max(vc2) as vc2,max(vc3) as vc3,max(vc4) as vc4 from \"IntStore\") as foo");

	while (true){
		std::array<int,4> tmp;
		{
			pqxx::work trans_causal(conn_causal);
			trans_causal.exec("set search_path to causalstore,public");
			auto r = trans_causal.prepared("selectit").exec();
			assert(r[0][0].to(tmp[0]));
			assert(r[0][1].to(tmp[1]));
			assert(r[0][2].to(tmp[2]));
			assert(r[0][3].to(tmp[3]));
			trans_causal.commit();
		}
		{
			pqxx::work trans_strong(conn_strong);
			trans_strong.exec("set search_path to \"BlobStore\",public");
			binarystring blob(&tmp[0],tmp.size()*4);
			auto r = trans_strong.prepared("Update")(blob).exec();
			assert(r.affected_rows() > 0);
			trans_strong.commit();
		}
		{
			pqxx::work trans_strong(conn_strong);
			trans_strong.exec("set search_path to \"BlobStore\",public");
			auto r = trans_strong.exec
				(string("select data from \"BlobStore\" where id = ") + id);
			binarystring bs
				(r[0][0]);
			std::array<int,4> arr;
			memcpy(&arr[0],bs.data(),tmp.size()*4);
			assert(arr[0] == tmp[0]);
			assert(arr[1] == tmp[1]);
			assert(arr[2] == tmp[2]);
			assert(arr[3] == tmp[3]);
		}
	}
	}
	catch(const pqxx::pqxx_exception &r){
		std::cerr << r.base().what() << std::endl;
		assert(false && "exec failed");
	}
}
