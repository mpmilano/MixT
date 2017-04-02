#pragma once
#include "batched_connection.hpp"
#include "DataStore.hpp"
#include "transaction_listener.hpp"

namespace myria{
	namespace server {

		template<typename Store, typename... phases>
		struct StoreRelay {
			static_assert(std::is_base_of<DataStore<typename Store::label>, Store >::value);

			std::function<std::unique_ptr<Store> ()> store;
			std::function<RemoteDeserialization_v ()> dsm;

			struct StoreSession : public ReceiverFun{
				std::unique_ptr<Store> store;
				DeserializationManager dsm;
				mutils::connection &c;
				moodycamel::BlockingReaderWriterQueue<std::vector<char> > queue;
				StoreSession(std::unique_ptr<Store> store,
										 RemoteDeserialization_v dsm,
										 DECT(c) &c)
					:store(std::move(store)),dsm(std::move(dsm)),c(c){}

				//events within thread
				void thread_loop(){
					while (!done){
						std::vector<char> msg;
						if (queue.wait_dequeue_timed(msg,10000)){
							auto selected_txn = ((txnID_t*) msg.data())[0];
							char const * const data = ((char*) msg.data()) + sizeof(txnID_t);
							whendebug(bool found_match = )
								(false || ... || phases::run_if_match(selected_txn,dsm,data));
							assert(found_match);
							struct fail{}; if (!found_match) throw fail{};
						}
					}
				}
				bool done{false};
				std::thread actions{thread_loop};

				//events outside of thread
				
				void deliver_new_event(std::size_t size, void const * const v){
					whendebug(bool b = ) queue.enqueue(std::vector<char>{(const char*) v, ((const char*) v) + size});
					assert(b);
				}
				void async_tick(){
					void;
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

			std::unique_ptr<mutils::rpc::ReceiverFun> start_session(whendebug(std::ofstream&,) mutils::connection& c ){
				return new StoreSession(store(), dsm(), c);
			}
			
			mutils::batched_connection::receiver receiver;
			StoreRelay(int port,std::function<std::unique_ptr<Store> ()> store, std::function<RemoteDeserialization_v ()> dsm)
				:store(std::move(store)),dsm(std::move(dsm)),receiver(port,start_session){}
		};

		template<typename Store, typename... transactions>
		using RelayForTransactions =
			StoreRelay<Store,listener_for<transactions, transactions::template find_phase<typename Store::label> >...>;
		
	}}
