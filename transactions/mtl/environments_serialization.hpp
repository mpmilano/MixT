#pragma once
#include "mutils-serialization/SerializationSupport.hpp"
#include "mutils-networking/local_connection.hpp"
#include "mutils-networking/additional_serialization_support.hpp"
#include "mtl/builtins.hpp"
#include "mtl/environments.hpp"

namespace myria{ namespace mtl{

		
		template<typename T, char... str>
		void serialize_holder(const type_holder<T,str...>& t, mutils::local_connection &c){
#ifndef NDEBUG
			whendebug(c.dump_bytes());
			using namespace mutils;
			const auto name = typename_str<DECT(t)>::f();
			c.send_data(name.size() + 1, name.c_str());
#endif
			whendebug(c.dump_bytes());
			c.send(t.t);
			whendebug(c.dump_bytes());
#ifndef NDEBUG
			c.send_data(name.size() + 1, name.c_str());
#endif
			whendebug(c.dump_bytes());
			c.send(t.curr_pos,t.bound);
			whendebug(c.dump_bytes());
#ifndef NDEBUG
			c.send_data(name.size() + 1, name.c_str());
#endif
			whendebug(c.dump_bytes());
		}

		template<typename T, typename DSM, char... str>
		void receive_holder(DSM *dsm, type_holder<T,str...>& t, mutils::local_connection &c){
#ifndef NDEBUG
			using namespace mutils;
			const auto my_name = typename_str<DECT(t)>::f();
			{
				char remote_name[my_name.size() + 1];
				c.receive_data(my_name.size() + 1, remote_name);
				if (remote_name != my_name){
					std::cout << remote_name << std::endl;
					std::cout << std::endl;
					std::cout << my_name << std::endl;
				}
				assert((remote_name == my_name));
			}
#endif
			auto t_p = mutils::from_bytes_noalloc<DECT(t.t)>(dsm,c.raw_buf());
			t.t = *t_p;
			c.mark_used(mutils::bytes_size(*t_p));
			
#ifndef NDEBUG
			{
				char remote_name[my_name.size() + 1];
				c.receive_data(my_name.size() + 1, remote_name);
				if (remote_name != my_name){
					std::cout << remote_name << std::endl;
					std::cout << std::endl;
					std::cout << my_name << std::endl;
				}
				assert((remote_name == my_name));
			}
#endif
			c.receive(t.curr_pos,t.bound);
#ifndef NDEBUG
			{
				char remote_name[my_name.size() + 1];
				c.receive_data(my_name.size() + 1, remote_name);
				if (remote_name != my_name){
					std::cout << remote_name << std::endl;
					std::cout << std::endl;
					std::cout << my_name << std::endl;
				}
				assert((remote_name == my_name));
			}
#endif
		}
		
		template<typename T, char... str>
		void serialize_holder(const value_holder<T,str...>& t, mutils::local_connection &c){
			whendebug(c.dump_bytes());
			c.send(whendebug(mutils::bytes_size(mutils::type_name<value_holder<T,str...> >()), mutils::type_name<value_holder<T,str...> >(),)
						 t.t
				);
			whendebug(c.dump_bytes());
		}
		
		template<typename T, typename DSM, char... str>
		void receive_holder(DSM *dsm, value_holder<T,str...>& t, mutils::local_connection &c){
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
			auto t_p = mutils::from_bytes_noalloc<DECT(t.t)>(dsm,c.raw_buf(),mutils::context_ptr<DECT(t.t)>{});
            t = *t_p;
			c.mark_used(mutils::bytes_size(*t_p));
		}


		template<typename T, char... str>
		void serialize_holder(const remote_holder<T,str...>& t, mutils::local_connection &c){
			whendebug(c.dump_bytes());
			c.send(whendebug(mutils::bytes_size(mutils::type_name<remote_holder<T,str...> >()), mutils::type_name<remote_holder<T,str...> >(),)
						 t.handle, t.curr_pos);
			whendebug(c.dump_bytes());
		}
		
		template<typename T>
		void serialize_holder(const remote_map_holder<T>& t, mutils::local_connection &c){
			whendebug(c.dump_bytes());
#ifndef NDEBUG
			whendebug(c.dump_bytes());
			c.send(t.is_initialized);
			whendebug(c.dump_bytes());
			auto map_name = mutils::typename_str<T>::f();
			char map_name_cstr[map_name.size() + 1];
			memcpy(map_name_cstr, map_name.c_str(),map_name.size());
			map_name_cstr[map_name.size()] = 0;
			c.get_log_file() << "This remote map holds " << map_name << std::endl;
			whendebug(c.dump_bytes());
			c.send_data(map_name.size() + 1,map_name_cstr);
			whendebug(c.dump_bytes());
			c.get_log_file() << "Sending " << t.super.size() << " entries in this remote map" << std::endl;
#endif
			whendebug(c.dump_bytes());
			c.send((std::size_t)t.super.size());
			whendebug(c.dump_bytes());
			for (const auto &p : t.super){
				whendebug(c.dump_bytes());
				c.send(p.first);
				whendebug(c.dump_bytes());
				serialize_holder(p.second, c);
				whendebug(c.dump_bytes());
#ifndef NDEBUG
				const std::size_t simple_nonce{141341313};
				whendebug(c.dump_bytes());
				c.send(simple_nonce);
				whendebug(c.dump_bytes());
#endif
			}
			whendebug(c.send_data(map_name.size() + 1,map_name_cstr));
			whendebug(c.dump_bytes());
		}
		
		template<typename T, typename... ctx>
		void receive_holder(mutils::DeserializationManager<ctx...> *dsm, remote_map_holder<T>& t, mutils::local_connection &c){
#ifndef NDEBUG
			c.receive(t.is_initialized);
			const auto holder_name = mutils::typename_str<T>::f();
			c.get_log_file() << "Expecting this remote map to hold " << holder_name << std::endl;
			{//nonce time
				char str1[holder_name.size()+1];
				c.receive_data(holder_name.size()+1,str1);
				c.get_log_file() << "Nonce reports " << str1 << std::endl;
				c.get_log_file().flush();
				assert(str1 == holder_name);
			}
#endif
			//receive map
			std::size_t map_size{0};
			c.receive(map_size);
			whendebug(c.get_log_file() << "Receiving " << map_size << " entries in this remote map" << std::endl);
			for (auto i = 0u; i < map_size; ++i){
				using first_t = typename DECT(t.super)::key_type;
				using second_t = typename DECT(t.super)::mapped_type;
				first_t key;
				c.receive(key);
				second_t &entry = t.super[key];
				receive_holder(dsm,entry,c);
#ifndef NDEBUG
				std::size_t simple_nonce{0};
				c.receive(simple_nonce);
				assert(simple_nonce == 141341313);
#endif
			}
			assert(t.super.size() == map_size);
#ifndef NDEBUG
			{//nonce time
				char str1[holder_name.size()+1];
				c.receive_data(holder_name.size()+1,str1);
				c.get_log_file() << "Nonce reports " << str1 << std::endl;
				c.get_log_file().flush();
				assert(str1 == holder_name);
			}
#endif
		}
		
		template<typename T, typename DSM, char... str>
		void receive_holder(DSM *dsm, remote_holder<T,str...>& t, mutils::local_connection &c){
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

		template<typename label, typename... T>
		void send_remote_maps(remote_map_aggregator<T...>& a, mutils::local_connection &c){
			constexpr std::size_t number_expected = ((std::is_same<typename T::label, label>::value ? 1 : 0) + ... + 0);
#ifndef NDEBUG
			c.get_log_file() << "Sending " << number_expected << " remote maps" << std::endl;
			c.send(number_expected);
#endif
			return ((std::is_same<typename T::label, label>::value ? serialize_holder<typename T::Handle_t>(a,c) : (void)a),...);
			//return (serialize_holder<typename T::Handle_t>(a,c), ...);
		}

		template<typename store, char... str>
		bool send_holder_values(mutils::String<str...> holder_name, store &s, mutils::local_connection &c,
														std::enable_if_t<!builtins::is_builtin<mutils::String<str...>>::value>* = nullptr){
			using holder = typename store::template find_holder_by_name<DECT(holder_name)>;
			holder& h = s;
			whendebug(c.dump_bytes());
			serialize_holder(h,c);
			whendebug(c.dump_bytes());
			return true;
		}
		
		template<typename label, typename store, typename... requires>
		void send_store_values(const mutils::typeset<requires...>&, store &s, mutils::local_connection &c){
#ifndef NDEBUG
			whendebug(c.dump_bytes());
			std::string nonce = mutils::type_name<mutils::typeset<requires...> >();
			c.send_data(nonce.size() + 1, nonce.c_str());
			c.get_log_file() << "about to send remote maps" << std::endl;
#endif
			whendebug(c.dump_bytes());
			send_remote_maps<label>(s.as_virtual_holder(),c);
			whendebug(c.dump_bytes());
#ifndef NDEBUG
			c.get_log_file() << "remote maps sent" << std::endl;
			c.send_data(nonce.size() + 1, nonce.c_str());
			c.get_log_file() << "now resending nonce: " << nonce << std::endl<< std::endl;
#endif
			whendebug(c.dump_bytes());
			auto worked = (send_holder_values(typename requires::name{}, s, c) && ... && true);
			whendebug(c.dump_bytes());
			assert(worked);
			(void)worked;
			whendebug(c.dump_bytes());
		}
		
		template<typename store, typename DSM, char... str>
		bool receive_holder_values(DSM* dsm, mutils::String<str...> holder_name, store &s, mutils::local_connection &c,
															 std::enable_if_t<!builtins::is_builtin<mutils::String<str...>>::value>* = nullptr){
			using holder = typename store::template find_holder_by_name<DECT(holder_name)>;
			holder& h = s;
			receive_holder(dsm,h,c);
			return true;
		}

		template<typename label, typename DSM, typename... T>
		void receive_remote_maps(DSM* dsm, remote_map_aggregator<T...>& a, mutils::local_connection &c){
			constexpr auto number_expected = ((std::is_same<typename T::label, label>::value ? 1 : 0) + ... + 0);
#ifndef NDEBUG
			c.get_log_file() << "Expecting " << number_expected << " remote maps" << std::endl;
			c.get_log_file() << "Remote expects to send: " << *c.receive<std::size_t>(dsm,sizeof(std::size_t)) << " remote maps" << std::endl;
#endif
			return ((std::is_same<typename T::label, label>::value ? receive_holder<typename T::Handle_t>(dsm,a,c) : (void)a),...);
			//return (receive_holder<typename T::Handle_t>(dsm,a,c),...);
		}
		
		template<typename label, typename store, typename DSM, typename... provides>
		void receive_store_values(DSM* dsm, const mutils::typeset<provides...>&, store &s, mutils::local_connection &c){
#ifndef NDEBUG
			std::string nonce = mutils::type_name<mutils::typeset<provides...> >();
			{
				char remote_nonce[nonce.size() + 1];
				remote_nonce[nonce.size()] = 0;
				c.receive_data(nonce.size() + 1, remote_nonce);
				if (nonce != remote_nonce){
					std::cout << nonce << std::endl << std::endl << std::endl << std::endl;
					std::cout << remote_nonce << std::endl;
				}
				assert(nonce == remote_nonce);
				c.get_log_file() << "First verification passed.  Now receiving remote maps" << std::endl;
			}
#endif
			receive_remote_maps<label>(dsm,s.as_virtual_holder(), c);
#ifndef NDEBUG
			{
				char remote[nonce.size() + 1];
				remote[nonce.size()] = 0;
				c.receive_data(nonce.size() + 1, remote);
				c.get_log_file() << "environment serialization nonce from remote: " << remote << std::endl
						 << std::endl;
				c.get_log_file() << "environment serialization nonce expected: " << nonce << std::endl;
				c.get_log_file().flush();
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
