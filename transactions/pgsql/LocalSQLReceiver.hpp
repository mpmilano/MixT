#pragma once
#include "LocalSQLConnection.hpp"
#include "LocalSQLTransaction.hpp"
#include "SQLConstants.hpp"
#include "dual_connection.hpp"
#include "pgexceptions.hpp"

namespace myria {
namespace pgsql {
namespace local {

using conn_space::receiver;

template <Level l> class SQLReceiver {
public:
  std::unordered_set<LocalSQLConnection_super *> unused_connections;
  mutils::dual_connection_receiver<receiver> r;

  using sizes_t = std::vector<std::size_t>;
	using resource_pool = mutils::ResourcePool<LocalSQLConnection<l>>;

  static mutils::dualstate_action_t
  new_connection(resource_pool &pool,
                 whendebug(std::ofstream &log_file, )::mutils::connection &data,
                 ::mutils::connection &control) {
    struct ReceiverFun : public mutils::dual_state_receiver {

			~ReceiverFun(){struct this_is_a_problem{}; throw this_is_a_problem{};}
			
      resource_pool &pool;
      whendebug(std::ofstream &log_file;)

			using weak_LocalSQLConnection = typename resource_pool::WeakResource;

			LocalSQLConnection<l> *tick_me{nullptr};
			int _underlying_fd{0};
			weak_LocalSQLConnection acquire_connection(){
				auto lr = pool.acquire();
				_underlying_fd = lr->underlying_fd();
				if (!lr.tick_is_handled) {
					lr.tick_is_handled = true;
					tick_me = &(*lr);
				}
				return lr;
			}
			
			weak_LocalSQLConnection db_connection{acquire_connection()};
      std::unique_ptr<LocalSQLTransaction<l>> current_trans{nullptr};

      mutils::connection &data_conn;
      mutils::connection &control_conn;
      const long int serialization_failure = mutils::long_rand();

      int underlying_fd() {
        return _underlying_fd;
      }

      void async_tick() {
        try {
          if (tick_me) {
            db_connection->tick();
          } else if (current_trans){
            current_trans->conn->tick();
          }
        } catch (const SerializationFailure &sf) {
          sf.control_conn.send(serialization_failure);
					ReceiverFun* other{nullptr}; sf.conn.current_receiver(&other);
					assert(other);
          other->db_connection = other->current_trans->store_abort(std::move(current_trans));
        } catch (const SQLFailure &sf) {
          std::cerr << sf.what() << std::endl;
          assert(false && "fatal SQL error");
          struct diedie {};
          throw diedie{};
        }
      }

      void deliver_new_data_event(const void *data) {
        const char *_data = (const char *)data;
        if (!current_trans) {

          if (_data[0] != 4 && _data[0] != 1) {
            std::cout << (int)_data[0] << std::endl;
          }
          assert(_data[0] == 4 || _data[0] == 1);

          if (_data[0] == 4) {
            assert(!current_trans);
            assert(db_connection);
            current_trans.reset(new LocalSQLTransaction<l>(
                std::move(db_connection) whendebug(, log_file)));
          } else {
// if we're aborting a non-existant transaction, there's nothing to do.
#ifndef NDEBUG
            log_file << "aborting non-existant transaction" << std::endl;
            log_file.flush();
#endif
          }
        } else if (_data[0] == 0) {
          // we're finishing this transaction
          db_connection =
              current_trans->store_commit(std::move(current_trans), data_conn);
          assert(db_connection);
        } else if (_data[0] == 1) {
          // we're aborting this transaction
          db_connection = current_trans->store_abort(std::move(current_trans));
          assert(db_connection);
        } else {
          assert(_data[0] != 4);
          assert(_data[0] == 3);
          TransactionNames name = *((TransactionNames *)(_data + 1));
          auto pair = current_trans->receiveSQLCommand(
              std::move(current_trans), name, _data + 1 + sizeof(name),
              data_conn);
          current_trans = std::move(pair.first);
          db_connection = std::move(pair.second);
          assert(current_trans || db_connection);
        }

#ifndef NDEBUG
        if (current_trans) {
          current_trans->log_file << "done processing this request"
                                  << std::endl;
          current_trans->log_file.flush();
        } else {
          log_file << "done processing this request; transaction was destroyed"
                   << std::endl;
        }
#endif
      }

      void deliver_new_control_event(const void *v) {
        (void)v;
        assert(*((long int *)v) == serialization_failure);
        data_conn.send(serialization_failure);
        const std::string why{"I do not know why this happened"};
        std::size_t size = mutils::bytes_size(why);
        control_conn.send(size);
        control_conn.send(why);
      }

      ReceiverFun(ReceiverFun &&o)
				: pool(o.pool),
					whendebug(log_file(o.log_file), )
                db_connection(std::move(o.db_connection)),
            current_trans(std::move(o.current_trans)), data_conn(o.data_conn),
            control_conn(o.control_conn) {}

      ReceiverFun(resource_pool& pool,
									whendebug(std::ofstream &log_file, )::mutils::connection &data,
          ::mutils::connection &control)
				: pool(pool),
					whendebug(log_file(log_file), ) data_conn(data),
            control_conn(control) {}
    };
    return mutils::dualstate_action_t{
			new ReceiverFun(pool,whendebug(log_file, ) data, control)};
  }

private:
  std::unique_ptr<resource_pool> make_rp() {
    auto &uc = this->unused_connections;
    return std::make_unique<resource_pool>(
	    (PREFERRED_DB_CONS),(SPARE_DB_CONS),
	    [&uc](){
				return new LocalSQLConnection<l>(uc);}, false/* no overdraws*/);
  }

public:
  SQLReceiver()
      : r((l == Level::strong ? strong_sql_port : causal_sql_port),
          [rp = make_rp()](auto &... args) {
            return new_connection(*rp, args...);
          }) {}
};
}
}
}
