#pragma once
#include "SQLStore.hpp"
#include "SQLConnection.hpp"
#include "backtrace.hpp"
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
        GDataStore& gstore;
	SQLStore_impl::SQLConnection& sql_conn;
	pqxx::work trans;
public:
	bool commit_on_delete = false;
        SQLTransaction(GDataStore& store, SQLStore_impl::SQLConnection& c)
            :gstore(store),sql_conn(c),trans(sql_conn.conn){
		assert(!sql_conn.in_trans);
		sql_conn.in_trans = true;
		sql_conn.current_trans = this;
		}

	bool is_serialize_error(const pqxx::pqxx_exception &r){
		auto s = std::string(r.base().what());
		return s.find("could not serialize access due to concurrent update") != std::string::npos;
	}
	
	SQLTransaction(const SQLTransaction&) = delete;

#define default_sqltransaction_catch									\
	catch(const pqxx::pqxx_exception &r){								\
		commit_on_delete = false;										\
		if (is_serialize_error(r)) throw Transaction::SerializationFailure{}; \
		else throw Transaction::CannotProceedError{r.base().what() + show_backtrace()}; \
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
                return gstore;
	}

	std::list<SQLStore_impl::GSQLObject*> objs;
	void add_obj(SQLStore_impl::GSQLObject* gso){
		objs.push_back(gso);
	}

	~SQLTransaction(){
		if (commit_on_delete) {
			commit();
		}
		else sql_conn.in_trans = false;
		sql_conn.current_trans = nullptr;
		for (auto gso : objs)
			if (gso->currentTransactionContext() == this)
				gso->setTransactionContext(nullptr);
	}
	
};
