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
	SQLConnection() = default;
	SQLConnection(const SQLConnection&) = delete;
};
