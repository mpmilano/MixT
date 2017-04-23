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

auto init_causal(){
	auto conn_causal = make_unique<pqxx::connection>();
	conn_causal->prepare("selectit","select a,a,a,a from (SELECT (extract(epoch from current_timestamp)*100000)::bigint as a) as t;");
	return conn_causal;
}

void select_causal_timestamp(pqxx::connection& conn_causal, std::array<long long,4>& tmp) {
	pqxx::work trans_causal(conn_causal);
	trans_causal.exec("set search_path to causalstore,public");
	auto r = trans_causal.prepared("selectit").exec();
        bool assrt = r[0][0].to(tmp[0]) && r[0][1].to(tmp[1]) && r[0][2].to(tmp[2]) && r[0][3].to(tmp[3]);
        assert(assrt);
	trans_causal.commit();
}
