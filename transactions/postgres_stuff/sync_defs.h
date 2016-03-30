#pragma once

#include <pqxx/pqxx>
#include <array>
#include <cassert>
#include <iostream>
#include <memory>

using namespace pqxx;
using namespace std;

const string id = to_string(
#include "../big_prime"
	);

auto init_strong(){
	auto conn_strong = make_unique<pqxx::connection>("host=128.84.217.149");
	conn_strong->prepare("Update",
						string("update \"BlobStore\" set data=$1,version=$2 where id= ") + id);
	return conn_strong;
}

auto init_causal(){
	auto conn_causal = make_unique<pqxx::connection>();
	conn_causal->prepare("selectit","select max(vc1) as vc1,max(vc2) as vc2,max(vc3) as vc3,max(vc4) as vc4 from (select max(vc1) as vc1,max(vc2) as vc2,max(vc3) as vc3,max(vc4) as vc4 from \"BlobStore\" union select max(vc1) as vc1,max(vc2) as vc2,max(vc3) as vc3,max(vc4) as vc4 from \"IntStore\") as foo");
	return conn_causal;
}

void select_causal_timestamp(pqxx::connection& conn_causal, std::array<int,4>& tmp) {
	pqxx::work trans_causal(conn_causal);
	trans_causal.exec("set search_path to causalstore,public");
	auto r = trans_causal.prepared("selectit").exec();
	assert(r[0][0].to(tmp[0]));
	assert(r[0][1].to(tmp[1]));
	assert(r[0][2].to(tmp[2]));
	assert(r[0][3].to(tmp[3]));
	trans_causal.commit();
}

void update_strong_clock(pqxx::connection& conn_strong, std::array<int,4>& tmp){
	pqxx::work trans_strong(conn_strong);
	trans_strong.exec("set search_path to \"BlobStore\",public");
	binarystring blob(&tmp[0],tmp.size()*4);
	auto r = trans_strong.prepared("Update")(blob)(tmp[0]+tmp[1]+tmp[2]+tmp[3]).exec();
	assert(r.affected_rows() > 0);
	trans_strong.commit();
}

void select_strong_clock(pqxx::connection& conn_strong, std::array<int,4>& arr){
	pqxx::work trans_strong(conn_strong);
	trans_strong.exec("set search_path to \"BlobStore\",public");
	auto r = trans_strong.exec
		(string("select data from \"BlobStore\" where id = ") + id);
	binarystring bs
		(r[0][0]);
	//for (int i : arr) std::cout << i << " ";
				//std::cout << std::endl;
	memcpy(&arr[0],bs.data(),arr.size()*4);
}

void verify_strong_clock(pqxx::connection& conn_strong, std::array<int,4>& tmp){
	std::array<int,4> arr;
	select_strong_clock(conn_strong, arr);
	assert(arr[0] == tmp[0]);
	assert(arr[1] == tmp[1]);
	assert(arr[2] == tmp[2]);
	assert(arr[3] == tmp[3]);
}
