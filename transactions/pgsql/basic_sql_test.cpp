#include "LocalSQLConnection.hpp"
#include "pgtransaction.hpp"
#include "pgexceptions.hpp"
#include "epoll.hpp"
#include <pqxx/pqxx>
using namespace myria;
using namespace pgsql;
using namespace local;
using namespace pqxx;
using namespace mutils;
int main(){
	const std::array<unsigned long long, 4> random_stuff{{24532453523,234514234234,32544675467,2464531413}};
	{
		pqxx::connection c;
		c.prepare("pqxx","update \"BlobStore\".\"BlobStore\" set data = $1 where id = 1;");
		pqxx::work trans{c};
		auto bytes = pqxx::binarystring{(char*) &random_stuff[0] ,sizeof(unsigned long long)*4};
		trans.prepared("pqxx")(bytes).exec();
		trans.commit();
	}
		try {
			auto _c = std::make_unique<LocalSQLConnection_super>();
			auto& c = *_c;
			using transaction = pgtransaction;
			const std::string name = "test_statement";
			{
				transaction trans{c,1};
				trans.template exec_async<std::function<void ()> >(
					[]{},
					"set search_path to \"BlobStore\",public;");
				trans.commit([]{});
			}
			std::cout << "on transaction 2" << std::endl;
			c.template prepare<long int>(name,"update  \"BlobStore\".\"IntStore\" set data=$1 where id = 1");
			c.template prepare<Bytes>("blobcheck","update  \"BlobStore\".\"BlobStore\" set data=$1 where id = 1");
			transaction trans{c,2};
			trans.template exec_async<std::function<void ()> >(
				[]{},
				"set search_path to \"BlobStore\",public;");
			long int fourty_two = 42;
			trans.template exec_prepared<std::function<void ()> >(
				[]{}, name, fourty_two);
			trans.template exec_async<std::function<void (long int)> >(
				[](long int r){
					std::cout << r << std::endl;
				},"select data from \"IntStore\" where id = 1;");
			trans.template exec_async<std::function<void (Bytes)> >(
				[&](Bytes b){
					assert(b.size == (sizeof(unsigned long long) * 4));
					std::array<unsigned long long, 4> more_random_stuff;
					memcpy(&more_random_stuff[0],b.bytes,b.size);
					assert(more_random_stuff == random_stuff);
					std::cout << "YAY byte order is maintained for blobs.  praise the lord." << std::endl;
				},"select data from \"BlobStore\" where id = 1");
			trans.commit([](){
					std::cout << "we committed; ALL PQ COMMANDS COMPLETE" << std::endl;
				});
			std::cout << "commit transaction 2" << std::endl;
			EPoll ep;
			std::function<void (EPoll&, LocalSQLConnection_super&)> f =
				[](EPoll&, LocalSQLConnection_super& c){
				//std::cout << "we ticked" << std::endl;
				c.tick();
			};
			std::cout << "waiting on server" << std::endl;
			ep.add(std::move(_c),f);
			c.tick();
			std::cout << "manual tick complete" << std::endl;
			//for (int i = 0; i < 10000; ++i) c.tick();
			for (int i = 0; i < 10000; ++i)
				ep.wait();
			std::cout << "fell through ep" << std::endl;
			sleep(4000);
		}
		catch(const std::exception& e){
			std::cout << e.what() << std::endl;
		}
}
