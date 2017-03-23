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
			{{
				cout_conn sock;
				std::unique_ptr<LocalSQLTransaction<Level::causal> > trans{
					new LocalSQLTransaction<Level::causal>{std::unique_ptr<LocalSQLConnection<Level::causal> >{
							new LocalSQLConnection<Level::causal>{std::cout}},}};
				{
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
					std::cout << "the value you are looking for is" << all_bits_used << endl;
				}
				auto conn = trans->store_commit(std::move(trans),sock);
				for (int i = 0; i < 10000; ++i)  conn->tick();
										
			}


				{
					char smuggled_out[4096];
					long int smuggled_out_int{0};
					cout_conn sock;
					std::unique_ptr<LocalSQLTransaction<Level::strong> > trans{
						new LocalSQLTransaction<Level::strong>{std::unique_ptr<LocalSQLConnection<Level::strong> >{
								new LocalSQLConnection<Level::strong>{std::cout}}}};
					{
						const long int sixty{60};
						const long int fouroseven{407};
						static_assert(sizeof(sixty) >= 8);
						{
							std::function<void (long int)> action = [](auto v){cout << "version: " << v << endl;};
							trans->select_version_s(action,Table::IntStore, 250);
						}
						{
							std::function<void (long int, mutils::Bytes)> action = [&smuggled_out](auto v, auto b){cout << "version: " << v << endl;
																																																		 assert(b.size <= 4096);
																																																		 b.to_bytes(smuggled_out);};
							trans->select_version_data_s(action,Table::BlobStore, 1);
						}
						{
							std::function<void (long int, long int)> action = [&smuggled_out_int](auto v, auto b){cout << "version: " << v << endl;
																																												smuggled_out_int = b;};
							trans->select_version_data_s(action,Table::IntStore, 250);
						}
						{
							std::function<void (long int)> action = [](auto v){cout << "version: " << v << endl;};
							trans->update_data_s(action,Table::IntStore, 250, (char*) &smuggled_out_int);
						}
						{
							std::function<void (long int)> action = [](auto v){cout << "version: " << v << endl;};
							trans->update_data_s(action,Table::BlobStore, 1, smuggled_out);
						}
						{
							
						}
						const long int all_bits_used = std::numeric_limits<long int>::max();
						const long int all_bits_used2 = std::numeric_limits<long int>::min();
						
						
						std::function<void (long int, long int, long int, long int)> action = [](long int i, long int j, long int k, long int l){cout << "selected " << i  << ", " << j << ", " << k << ", " << l;};
					
						trans->prepared(action,*trans->conn,LocalTransactionNames::usertest1,"select $1::bigint as constant, $2::bigint as r2, $3::bigint as r3, $4::bigint as r4 from \"IntStore\" where not id = $2::bigint limit 1;",all_bits_used,all_bits_used2,sixty,fouroseven);
						std::cout << "the value you are looking for is" << all_bits_used << endl;
					}
					for (int i = 0; i < 1000; ++i) trans->conn->tick();
					trans->obj_exists(250, sock);
					auto conn = trans->store_commit(std::move(trans),sock);
					while(true) conn->tick();
					
					return 0;
			}}
			
		}}}



int main(){
	try {
		return myria::pgsql::local::real_main();
	} catch(exception& e){
		cout << e.what() << endl;
	}
}
