#pragma once
#include "batched_connection.hpp"
#include "DataStore.hpp"
#include "transaction_listener.hpp"

namespace myria{
	namespace server {

		template<typename Store, typename... phases>
		struct StoreRelay {
			static_assert(std::is_base_of<DataStore<typename Store::label>, Store >::value);

			struct captive_store{
				virtual Store& store() = 0;
				virtual mutils::DeserializationManager &dsm() = 0;
				virtual ~captive_store() = default;
			};
			
			std::function<std::unique_ptr<captive_store> ()> new_connection;

			struct StoreSession : public mutils::rpc::ReceiverFun{
				std::unique_ptr<captive_store> cstore;
				Store& store{cstore->store()};
				mutils::DeserializationManager &dsm{cstore->dsm()};
				mutils::connection &c;
				moodycamel::BlockingReaderWriterQueue<std::vector<char> > queue;
				StoreSession(std::unique_ptr<captive_store> cstore,DECT(c) &c)
					:cstore(std::move(cstore)),c(c),actions{[this]{this->thread_loop();}}{}

				//events within thread
				void thread_loop(){
					while (!done){
						std::vector<char> msg;
						if (queue.wait_dequeue_timed(msg,10000)){
							auto selected_txn = ((txnID_t*) msg.data())[0];
							char const * const data = ((char*) msg.data()) + sizeof(txnID_t);
							whendebug(bool found_match = )
								(false || ... || phases::run_if_match(selected_txn,dsm,c,data));
							assert(found_match);
							struct fail{}; if (!found_match) throw fail{};
						}
					}
				}
				bool done{false};
				std::thread actions;

				//events outside of thread
				
				void deliver_new_event(std::size_t size, void const * const v){
					whendebug(bool b = ) queue.enqueue(std::vector<char>{(const char*) v, ((const char*) v) + size});
					assert(b);
				}
				void async_tick(){
				}
				int underlying_fd(){
					static mutils::eventfd fd;
					return fd.underlying_fd();
				}

				~StoreSession(){
					done = true;
					actions.join();
				}
				
			};

			std::unique_ptr<mutils::rpc::ReceiverFun> start_session(whendebug(std::ostream&,) mutils::connection& c ){
				return std::unique_ptr<mutils::rpc::ReceiverFun>{new StoreSession(new_connection(), c)};
			}
			mutils::rpc::new_connection_t start_session_wrapper =
				std::bind(&StoreRelay::start_session,this,std::placeholders::_1 whendebug(, std::placeholders::_2));
			
			mutils::batched_connection::receiver receiver;
			StoreRelay(int port,DECT(new_connection) new_connection)
				:new_connection(std::move(new_connection)),receiver(port,start_session_wrapper){}
		};

		template<typename Store, typename... transactions>
		using RelayForTransactions =
			StoreRelay<Store,listener_for<transactions, typename transactions::template find_phase<typename Store::label> >...>;
		
	}}
