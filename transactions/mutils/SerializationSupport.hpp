#pragma once
#include "utils.hpp"
#include "type_utils.hpp"
#include "SerializationMacros.hpp"
#include "macro_utils.hpp"
#include <vector>

namespace mutils{


	struct ByteRepresentable {
		virtual int to_bytes(char* v) const = 0;
		virtual int bytes_size() const = 0;
		virtual ~ByteRepresentable(){}
		//needs to exist, but can't declare virtual statics
		//virtual static T* from_bytes(char *v) const  = 0;
	};


	//end forward-declaring


	int to_bytes(const ByteRepresentable& b, char* v);

	int bytes_size(const ByteRepresentable& b);

	int to_bytes(const std::string& b, char* v);

	int bytes_size(const std::string& b);

	template<typename T>
	int to_bytes(const std::vector<T> &vec, char* _v){
		((int*)_v)[0] = vec.size();
		char* v = _v + sizeof(int);
		if (std::is_trivially_copyable<T>::value){
			int size = vec.size() * bytes_size(vec.back());
			memcpy(v, vec.data(),size);
			return size + sizeof(int);
		}
		else{
			int offset = 0;
			for (auto &e : vec){
				offset += (to_bytes(e,v + offset));
			}
			return offset + sizeof(int);
		}
	}

	template<typename T>
	int bytes_size (const std::vector<T> &v){
		if (std::is_trivially_copyable<T>::value)
			return v.size() * bytes_size(v.back()) + sizeof(int);
		else {
			int accum = 0;
			for (auto &e : v) accum += bytes_size(e);
			return accum + sizeof(int);
		}
	}


	template<typename T, restrict(std::is_trivially_copyable<T>::value)>
	int to_bytes(const T &t, char* v){
		auto res = memcpy(v,&t,sizeof(T));
		assert(res);
		return sizeof(T);
	}

	template<typename T, restrict2(std::is_trivially_copyable<T>::value)>
	auto bytes_size(const T&){
		return sizeof(T);
	}
	
	template<typename T, typename V>
	int to_bytes(const std::pair<T,V> &pair, char* v){
		auto offset = to_bytes(pair.first,v);
		auto offset2 = to_bytes(pair.second,v + offset);
		return offset + offset2;
	}

	template<typename T, typename V>
	int bytes_size (const std::pair<T,V> &pair){
		return bytes_size(pair.first) + bytes_size(pair.second);
	}

	template<typename T,
			 restrict(std::is_base_of<ByteRepresentable CMA T>::value)>
	std::unique_ptr<T> from_bytes(char const *v){
		return T::from_bytes(v);
	}


	template<typename T,
			 restrict2(std::is_trivially_copyable<T>::value)>
	std::unique_ptr<std::decay_t<T> > from_bytes(char const *v){
		using T2 = std::decay_t<T>;
		if (v) {
			auto t = std::make_unique<T2>(*(T2*)v);
			//std::memcpy(t.get(),v,sizeof(T));
			return std::move(t);
		}
		else return nullptr;
	}

	template<typename T>
	std::unique_ptr<T> from_bytes_stupid(T* t, char const * v) {
		return from_bytes<T>(v);
	}

	template<typename T>
	int to_bytes(const std::set<T>& s, char* _v){
		((int*)_v)[0] = s.size();
		char *v = _v + sizeof(int);
		for (auto &a : s){
			v += to_bytes(a,v);
		}
		return _v - v;
	}

	template<typename T>
	int bytes_size(const std::set<T>& s){
		int size = sizeof(int);
		for (auto &a : s) {
			size += bytes_size(a);
		}
		return size;
	}

	template<typename>
	struct is_set : std::false_type {};

	template<typename T>
	struct is_set<std::set<T> > : std::true_type {};

	template<typename>
	struct is_pair : std::false_type {};

	template<typename T, typename U>
	struct is_pair<std::pair<T,U> > : std::true_type {};

	template<typename>
	struct is_string : std::false_type {};

	template<>
	struct is_string<std::string> : std::true_type {};

	template<>
	struct is_string<const std::string> : std::true_type {};
	
	template<typename T>
	std::unique_ptr<type_check<is_string,T> > from_bytes(char const *v){
		return std::make_unique<T>(v);
	}
	
	template<typename T>
	std::unique_ptr<type_check<is_set,T> > from_bytes(char* const _v) {
		int size = ((int*)_v)[0];
		char* v = _v + sizeof(int);
		auto r = std::make_unique<std::set<typename T::key_type> >();
		for (int i = 0; i < size; ++i){
			auto e = from_bytes<typename T::key_type>(v);
			v += bytes_size(*e);
			r->insert(*e);
		}
		return std::move(r);
	}

	template<typename T>
	std::unique_ptr<type_check<is_pair,T > > from_bytes(char const * v){
		using ft = typename T::first_type;
		using st = typename T::second_type;
		auto fst = from_bytes<ft>(v);
		return std::make_unique<std::pair<ft,st> >
			(*fst, *from_bytes<st>(v + bytes_size(*fst)));
	}

	template<typename T>
	std::enable_if_t<is_vector<T>::value,std::unique_ptr<T> > from_bytes(char const * v){
		using member = typename T::value_type;
		if (std::is_trivially_copyable<typename T::value_type>::value){
			member const * const start = (member*) (v + sizeof(int));
			const int size = ((int*)v)[0];
			return std::unique_ptr<T>{new T{start, start + size}};
		}
		else{
			int size = ((int*)v)[0];
			auto* v2 = v + sizeof(int);
			int per_item_size = -1;
			std::unique_ptr<std::vector<member> > accum{new T()};
			for(int i = 0; i < size; ++i){
				std::unique_ptr<member> item = from_bytes<typename T::value_type>(v2 + (i * per_item_size));
				if (per_item_size == -1)
					per_item_size = bytes_size(*item);
				accum->push_back(*item);
			}
			return accum;
		}
	}
}
