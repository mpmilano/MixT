#pragma once
#include "SQLStore.hpp"
#include <pqxx/pqxx>

struct SQLTransaction;

struct SQLStore_impl::SQLConnection {
	bool in_trans = false;
	SQLTransaction* current_trans = nullptr;
	
	//hoping specifying nothing means
	//env will be used.
	pqxx::connection conn;
    SQLConnection(int ip):conn{std::string("host=") + string_of_ip(ip)}{}
	SQLConnection(const SQLConnection&) = delete;
};
