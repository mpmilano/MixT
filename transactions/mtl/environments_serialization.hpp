#pragma once
#include "SerializationSupport.hpp"
#include "local_connection.hpp"
#include "additional_serialization_support.hpp"

namespace myria{ namespace mtl{

		
		template<typename T, char... str>
		void serialize_holder(const type_holder<T,str...>& t, mutils::local_connection &c){
			c.send(whendebug(mutils::bytes_size(mutils::type_name<type_holder<T,str...> >()), mutils::type_name<type_holder<T,str...> >(),)
						 t.t,t.curr_pos,t.bound
				);
		}

		template<typename T, char... str>
		void receive_holder(mutils::DeserializationManager *dsm, type_holder<T,str...>& t, mutils::local_connection &c){
#ifndef NDEBUG
			DECT(mutils::bytes_size(std::string{})) remote_name_size;
			c.receive(remote_name_size);
			auto remote_name = c.receive<std::string>(dsm,remote_name_size);
			auto my_name = mutils::type_name<type_holder<T,str...> >();
			if (*remote_name != my_name){
				std::cout << *remote_name << std::endl;
				std::cout << std::endl;
				std::cout << my_name << std::endl;
			}
			assert((*remote_name == my_name));
#endif
			auto t_p = mutils::from_bytes_noalloc<DECT(t.t)>(dsm,c.raw_buf());
			t.t = *t_p;
			c.mark_used(mutils::bytes_size(*t_p));
			c.receive(t.curr_pos,t.bound);
		}
		
		template<typename T, char... str>
		void serialize_holder(const value_holder<T,str...>& t, mutils::local_connection &c){
			c.send(whendebug(mutils::bytes_size(mutils::type_name<value_holder<T,str...> >()), mutils::type_name<value_holder<T,str...> >(),)
						 t.t
				);
		}
		
		template<typename T, char... str>
		void receive_holder(mutils::DeserializationManager *dsm, value_holder<T,str...>& t, mutils::local_connection &c){
#ifndef NDEBUG
			DECT(mutils::bytes_size(std::string{})) remote_name_size;
			c.receive(remote_name_size);
			auto remote_name = c.receive<std::string>(dsm,remote_name_size);
			auto my_name = mutils::type_name<value_holder<T,str...> >();
			if (*remote_name != my_name){
				std::cout << *remote_name << std::endl;
				std::cout << std::endl;
				std::cout << my_name << std::endl;
			}
			assert((*remote_name == my_name));
#endif
			auto t_p = mutils::from_bytes_noalloc<DECT(t.t)>(dsm,c.raw_buf());
			t.t = *t_p;
			c.mark_used(mutils::bytes_size(*t_p));
		}


		template<typename T, char... str>
		void serialize_holder(const remote_holder<T,str...>& t, mutils::local_connection &c){
			serialize_holder(t.super,c);
			c.send(whendebug(mutils::bytes_size(mutils::type_name<remote_holder<T,str...> >()), mutils::type_name<remote_holder<T,str...> >(),)
						 t.initialized,t.list_usable, t.handle);
		}
		
		template<typename T, char... str>
		void receive_holder(mutils::DeserializationManager *dsm, remote_holder<T,str...>& t, mutils::local_connection &c){
			receive_holder(dsm,t.super,c);
#ifndef NDEBUG
			DECT(mutils::bytes_size(std::string{})) remote_name_size;
			c.receive(remote_name_size);
			auto remote_name = c.receive<std::string>(dsm,remote_name_size);
			auto my_name = mutils::type_name<remote_holder<T,str...> >();
			if (*remote_name != my_name){
				std::cout << *remote_name << std::endl;
				std::cout << std::endl;
				std::cout << my_name << std::endl;
			}
			assert((*remote_name == my_name));
#endif			
			c.receive(t.initialized,t.list_usable);
			auto hndl = mutils::from_bytes<DECT(t.handle)>(dsm,c.raw_buf());
			c.mark_used(mutils::bytes_size(*hndl));
			t.handle = std::move(*hndl);
		}

		template<typename store, char... str>
		bool send_holder_values(mutils::String<str...> holder_name, store &s, mutils::local_connection &c){
			using holder = typename store::template find_holder_by_name<DECT(holder_name)>;
			holder& h = s;
			serialize_holder(h,c);
			return true;
		}
		
		template<typename store, typename... requires>
		void send_store_values(const mutils::typeset<requires...>&, store &s, mutils::local_connection &c){
#ifndef NDEBUG
			std::string nonce = mutils::type_name<mutils::typeset<requires...> >();
			c.send(mutils::bytes_size(nonce),nonce);
#endif
			auto worked = (send_holder_values(typename requires::name{}, s, c) && ... && true);
			assert(worked);
			(void)worked;
		}
		
		template<typename store, char... str>
		bool receive_holder_values(mutils::DeserializationManager* dsm, mutils::String<str...> holder_name, store &s, mutils::local_connection &c){
			using holder = typename store::template find_holder_by_name<DECT(holder_name)>;
			holder& h = s;
			receive_holder(dsm,h,c);
			return true;
		}
		
		template<typename store, typename... provides>
		void receive_store_values(mutils::DeserializationManager* dsm, const mutils::typeset<provides...>&, store &s, mutils::local_connection &c){
#ifndef NDEBUG
			std::string nonce = mutils::type_name<mutils::typeset<provides...> >();
			auto nonce_size = mutils::bytes_size(nonce);
			c.receive(nonce_size);
			auto remote = *c. template receive<std::string>(nullptr,nonce_size);
			assert(nonce == remote);
#endif
			auto worked = (receive_holder_values(dsm,typename provides::name{}, s, c) && ... && true);
			assert(worked);
			(void)worked;
		}
	}}
