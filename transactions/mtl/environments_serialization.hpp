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
			c.send(whendebug(mutils::bytes_size(mutils::type_name<remote_holder<T,str...> >()), mutils::type_name<remote_holder<T,str...> >(),)
						 t.handle, t.curr_pos);
		}
		
		template<typename T>
		void serialize_holder(const remote_map_holder<T>& t, mutils::local_connection &c){
			whendebug(c.send(t.is_initialized));
			c.send((std::size_t)t.super.size());
			for (const auto &p : t.super){
				c.send(p.first);
				serialize_holder(p.second, c);
			}
		}
		
		template<typename T>
		void receive_holder(mutils::DeserializationManager *dsm, remote_map_holder<T>& t, mutils::local_connection &c){
			whendebug(c.receive(t.is_initialized));
			//receive map
			std::size_t map_size{0};
			c.receive(map_size);
			for (auto i = 0u; i < map_size; ++i){
				using first_t = typename DECT(t.super)::key_type;
				using second_t = typename DECT(t.super)::mapped_type;
				first_t key;
				c.receive(key);
				second_t &entry = t.super[key];
				receive_holder(dsm,entry,c);
			}
		}
		
		template<typename T, char... str>
		void receive_holder(mutils::DeserializationManager *dsm, remote_holder<T,str...>& t, mutils::local_connection &c){
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
			auto hndl = mutils::from_bytes<DECT(t.handle)>(dsm,c.raw_buf());
			c.mark_used(mutils::bytes_size(*hndl));
			t.handle = std::move(*hndl);
			c.receive(t.curr_pos);
		}

		template<typename... T>
		void send_remote_maps(remote_map_aggregator<T...>& a, mutils::local_connection &c){
			return (serialize_holder<typename T::Handle_t>(a,c), ...);
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
			send_remote_maps(s.as_virtual_holder(),c);
#ifndef NDEBUG
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

		template<typename... T>
		void receive_remote_maps(mutils::DeserializationManager* dsm, remote_map_aggregator<T...>& a, mutils::local_connection &c){
			return (receive_holder<typename T::Handle_t>(dsm,a,c),...);
		}
		
		template<typename store, typename... provides>
		void receive_store_values(mutils::DeserializationManager* dsm, const mutils::typeset<provides...>&, store &s, mutils::local_connection &c){
#ifndef NDEBUG
			std::string nonce = mutils::type_name<mutils::typeset<provides...> >();
			auto nonce_size = mutils::bytes_size(nonce);
			c.receive(nonce_size);
			{
				auto remote = *c. template receive<std::string>(nullptr,nonce_size);
				if (nonce != remote){
					std::cout << nonce << std::endl << std::endl << std::endl << std::endl;
					std::cout << remote << std::endl;
				}
				assert(nonce == remote);
			}
#endif
			receive_remote_maps(dsm,s.as_virtual_holder(), c);
#ifndef NDEBUG
			{
				auto remote = *c. template receive<std::string>(nullptr,nonce_size);
				if (nonce != remote){
					std::cout << nonce << std::endl << std::endl << std::endl << std::endl;
					std::cout << remote << std::endl;
				}
				assert(nonce == remote);
			}
#endif
			auto worked = (receive_holder_values(dsm,typename provides::name{}, s, c) && ... && true);
			assert(worked);
			(void)worked;
		}
	}}
