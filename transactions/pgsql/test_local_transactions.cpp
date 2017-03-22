#include <iostream>
#include "LocalSQLTransaction.hpp"
#include "LocalSQLConnection.hpp"

using namespace std; using namespace mutils;

namespace myria { namespace pgsql { namespace local{

			struct cout_conn : public connection {
			
				bool valid() const { return true; }
				std::size_t raw_receive(std::size_t , std::size_t const * const , void ** ) {
					assert(false && "not with this connection");
				}
				std::size_t raw_send(std::size_t how_many, std::size_t const * const sizes, void const * const * const bufs){
					std::cout << "payload as a string: ";
					std::size_t accum{0};
					for (std::size_t i = 0; i < how_many; ++i){
						auto size = sizes[i];
						char buf[size+1];
						buf[size] = 0;
						memcpy(buf,bufs[i],size);
						cout << buf << " :::: ";
						accum += size;
					}
					return accum;
				}
				std::ostream& get_log_file() { return cout;}	
			};
			
			int real_main()
			{
				cout_conn sock;
				std::unique_ptr<LocalSQLTransaction<Level::causal> > trans{
					new LocalSQLTransaction<Level::causal>{std::unique_ptr<LocalSQLConnection<Level::causal> >{
							new LocalSQLConnection<Level::causal>{std::cout}},
							std::cout}};
					const long int sixty{60};
					const long int fouroseven{407};
					static_assert(sizeof(sixty) >= 8);
/*
					trans->update_data_c([](long int, long int, long int, long int){cout << "Action complete" << endl;},Table::IntStore,1,250, std::array<int,NUM_CAUSAL_GROUPS>{{0,0,0,0}},(const char*)&sixty);
					trans->update_data_c([](long int, long int, long int, long int){cout << "Action complete" << endl;},Table::IntStore,1,250, std::array<int,NUM_CAUSAL_GROUPS>{{0,0,0,0}},(const char*)&sixty);
					trans->update_data_c([](long int, long int, long int, long int){cout << "Action complete" << endl;},Table::IntStore,1,250, std::array<int,NUM_CAUSAL_GROUPS>{{0,0,0,0}},(const char*)&sixty);
					trans->update_data_c([](long int, long int, long int, long int){cout << "Action complete" << endl;},Table::IntStore,1,250, std::array<int,NUM_CAUSAL_GROUPS>{{0,0,0,0}},(const char*)&sixty);
					//*/

					const long int all_bits_used = std::numeric_limits<long int>::max();
					const long int all_bits_used2 = std::numeric_limits<long int>::min();
					
					
					std::function<void (long int, long int, long int, long int)> action = [](long int i, long int j, long int k, long int l){cout << "selected " << i  << ", " << j << ", " << k << ", " << l;};
					
					trans->prepared(action,*trans->conn,LocalTransactionNames::usertest1,"select $1::bigint as constant, $2::bigint as r2, $3::bigint as r3, $4::bigint as r4 from \"IntStore\" where not id = $2::bigint and not vc1=$3::bigint limit 1;",all_bits_used,all_bits_used2,sixty,fouroseven);
					auto conn = trans->store_commit(std::move(trans),sock);
					std::cout << "the value you are looking for is" << all_bits_used << endl;
					while(true) conn->tick();
										
					return 0;
			}
		}}}



int main(){
	try {
		return myria::pgsql::local::real_main();
	} catch(exception& e){
		cout << e.what() << endl;
	}
}
