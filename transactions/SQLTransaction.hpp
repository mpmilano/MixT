#pragma once
#include "SQLStore.hpp"
#include "SQLConnection.hpp"
#include <pqxx/pqxx>

template<typename E>
auto exec_prepared_hlpr(E &e){
	return e.exec();
}

template<typename E, typename A, typename... B>
auto exec_prepared_hlpr(E &e, A&& a, B && ... b){
	auto fwd = e(std::forward<A>(a));
	return exec_prepared_hlpr(fwd,std::forward<B>(b)...);
}


struct SQLTransaction : public TransactionContext {
private:
	SQLStore::SQLConnection& sql_conn;
	pqxx::work trans;
public:
	bool commit_on_delete = false;
	SQLTransaction(SQLStore::SQLConnection& c):sql_conn(c),trans(sql_conn.conn){
		assert(!sql_conn.in_trans);
		sql_conn.in_trans = true;
		sql_conn.current_trans = this;
		}
	
	SQLTransaction(const SQLTransaction&) = delete;

#define default_sqltransaction_catch					\
	catch(const pqxx::pqxx_exception &r){				\
		commit_on_delete = false;						\
		std::cerr << r.base().what() << std::endl;		\
		assert(false && "exec failed");					\
	}

	
	template<typename Arg1, typename... Args>
	auto prepared(const std::string &name, const std::string &stmt,
				  Arg1 && a1, Args && ... args){
		try{
			sql_conn.conn.prepare(name,stmt);
			auto fwd = trans.prepared(name)(std::forward<Arg1>(a1));
			return exec_prepared_hlpr(fwd,std::forward<Args>(args)...);
		}
		default_sqltransaction_catch
	}


	auto exec(const std::string &str){
		try{
			return trans.exec(str);
		}
		default_sqltransaction_catch
	}
	
	bool commit() {
		sql_conn.in_trans = false;
		sql_conn.current_trans = nullptr;
		trans.commit();
		return true;
	}

	GDataStore& store() {
		return SQLStore::inst();
	}

	std::list<SQLStore::GSQLObject*> objs;
	void add_obj(SQLStore::GSQLObject* gso){
		objs.push_back(gso);
	}

	~SQLTransaction(){
		if (commit_on_delete) {
			std::cout << "commit on delete" << std::endl;
			commit();
		}
		else sql_conn.in_trans = false;
		sql_conn.current_trans = nullptr;
		for (auto gso : objs)
			if (gso->currentTransactionContext() == this)
				gso->setTransactionContext(nullptr);
	}
	
};
