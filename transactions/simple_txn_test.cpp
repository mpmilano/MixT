#include "SQLStore.hpp"
#include "transaction.hpp"
#include "transaction_macros.hpp"
#include "test_utils.hpp"
#include "threaded_trial.hpp"
#include "FinalHeader.hpp"
#include "configuration_params.hpp"

using namespace myria;
using namespace server;
using namespace pgsql;
using namespace mtl;
using namespace std;
using namespace chrono;
using namespace mutils;

namespace myria {

template <Level l> void client::txn_read() {
  auto &store = get_store<l>();
  auto hndl = store.template existingObject<int>(nullptr, get_name_read(0.5));
  using Hndl = DECT(hndl);
  constexpr auto read_trans = TRANSACTION(150 + Hndl::label::int_id::value,
                                          let remote x = hndl in{})::WITH(hndl);
  return read_trans.run_optimistic(trk, &dsm, *get_relay<l>().lock(), hndl);
  // return read_trans.run_local(hndl);
}

template <Level l> void client::txn_write() {
  auto &store = get_store<l>();
  auto hndl = store.template existingObject<int>(nullptr, get_name_write());
  using Hndl = DECT(hndl);
  constexpr auto incr_trans =
      TRANSACTION(Hndl::label::int_id::value,
                  let remote x = hndl in{x = x + 1})::WITH(hndl);
  return incr_trans.run_optimistic(trk, &dsm, *get_relay<l>().lock(), hndl);
  // return incr_trans.run_local(hndl);
}

run_result client::client_action(run_result &result) {
  auto &params = t.params;
  Level l = (mutils::better_rand() > params.percent_causal ? Level::strong
                                                           : Level::causal);
  bool write = mutils::better_rand() > params.percent_read;
  result.l = l;
  result.is_write = write;
  try {
    switch (l) {
    case Level::strong:
      if (write)
        txn_write<Level::strong>();
      else
        txn_read<Level::strong>();
      break;
    case Level::causal:
      if (write)
        txn_write<Level::causal>();
      else
        txn_read<Level::causal>();
      break;
    default:
      assert(false);
    };
    result.is_abort = false;
  } catch (const SerializationFailure &f) {
    result.is_abort = true;
		result.abort_string = f.what();
  }
  result.stop_time = high_resolution_clock::now();
  return result;
}
}

int main(int argc, char **argv) {
  configuration_parameters params;
  assert(argc == 1 || argc == 14);
  if (argc == 1) {
    std::cin >> params;
  } else read_from_args(params,argv + 1);
  std::cout << params << std::endl;
  test t1{params};
  t1.run_test();
	exit(0);
}
