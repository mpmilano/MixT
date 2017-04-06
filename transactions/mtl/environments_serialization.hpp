#pragma once
#include "SerializationSupport.hpp"
#include "local_connection.hpp"
#include "additional_serialization_support.hpp"

namespace myria{ namespace mtl{

		
		template<typename T, char... str>
		void serialize_holder(const type_holder<T,str...>& t, mutils::local_connection &c){
			c.send(whendebug(mutils::bytes_size(mutils::type_name<type_holder<T,str...> >()), mutils::type_name<type_holder<T,str...> >(),)
						 t.t,t.curr_pos,t.rollback_size,t.bound
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
			c.receive(t.curr_pos,t.rollback_size,t.bound);
		}
		
		template<typename T, char... str>
		void serialize_holder(const value_holder<T,str...>& t, mutils::local_connection &c){
			c.send(whendebug(mutils::bytes_size(mutils::type_name<value_holder<T,str...> >()), mutils::type_name<value_holder<T,str...> >(),)
						 t.pre_phase_t,t.t
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
			auto pre_phase = mutils::from_bytes_noalloc<DECT(t.pre_phase_t)>(dsm,c.raw_buf());
			t.pre_phase_t = *pre_phase;
			c.mark_used(mutils::bytes_size(*pre_phase));
			auto t_p = mutils::from_bytes_noalloc<DECT(t.t)>(dsm,c.raw_buf());
			t.t = *t_p;
			c.mark_used(mutils::bytes_size(*t_p));
		}


		template<typename T, char... str>
		void serialize_holder(const remote_holder<T,str...>& t, mutils::local_connection &c){
			serialize_holder(t.super,c);
			c.send(whendebug(mutils::bytes_size(mutils::type_name<remote_holder<T,str...> >()), mutils::type_name<remote_holder<T,str...> >(),)
						 t.initialized,t.list_usable,mutils::bytes_size(t.handle), t.handle);
		}
		
		template<typename T, char... str>
		void receive_holder(mutils::DeserializationManager *dsm, const remote_holder<T,str...>& t, mutils::local_connection &c){
			receive_holder(t.super,c);
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
			c.receive(t.initialized,t.list_usable,t.handle);
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
			auto worked = (receive_holder_values(dsm,typename provides::name{}, s, c) && ... && true);
			assert(worked);
			(void)worked;
		}
	}}
