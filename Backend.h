#pragma once
#include <list>
#include <memory>
#include <vector>
#include <queue>
#include <cassert>
#include <iostream>

namespace backend {
	enum class Level { causal, strong, fastest};

	template <Level L>
	class DataStore {

		
	private: 
		class HandlePrime;
		std::vector<std::unique_ptr<HandlePrime>> hndls;
		std::queue<int> next_ids;
		bool destructing = false;
		int get_next_id(){
			if (next_ids.size() > 0) {
				const int &ret = next_ids.front();
				next_ids.pop();
				return ret;
			}
			else return hndls.size();
		}
		class HandlePrime {
		private: 
			DataStore& parent;
			virtual bool is_abstract() = 0;
		public:
			const uint id;
			HandlePrime(DataStore& parent):parent(parent),id(parent.get_next_id()){}
			HandlePrime(const HandlePrime&) = delete;
			virtual ~HandlePrime() {
				if (!parent.destructing){
					parent.next_ids.push(id);
				}
			}
		};

		void place_correctly(std::unique_ptr<HandlePrime> h){
			if (h->id == hndls.size()) {
				hndls.push_back(std::move(h));
			}
			else {
				assert (h->id < hndls.size());
				assert (! hndls[h->id]);
				hndls[h->id] = std::move(h);
			}

		}

	public:
		template<typename T> 
		class Handle : public HandlePrime {
		private:
			std::unique_ptr<T> stored_obj;
			virtual bool is_abstract()  {return false;}
			operator T& (){ return *stored_obj;}
			void operator =(std::unique_ptr<T> n) {stored_obj = std::move(n);}
			operator std::unique_ptr<T>() {return std::move(stored_obj);}
			Handle(DataStore& parent,std::unique_ptr<T> n):HandlePrime(parent),stored_obj(std::move(n)){}
		public: 
			friend class DataStore;
			Handle (const Handle&) = delete;
			virtual ~Handle() {}
			
		};

		template<typename T> 
		Handle<T>& newHandle() {
			std::unique_ptr<Handle<T> > tmp(new Handle<T>(*this,std::unique_ptr<T>()));
			auto &ret = *tmp; 
			place_correctly(std::move(tmp));
			return ret;
		}

		template<typename T>
		Handle<T>& newHandle(std::unique_ptr<T> r){
			std::unique_ptr<Handle<T> > tmp(new Handle<T>(*this,std::move(r)));
			auto &ret = *tmp;
			place_correctly(std::move(tmp));
			return ret;
		}

		template<typename T>
		Handle<T>& newHandle(T r){
			auto obj = new T(r); 
			std::unique_ptr<Handle<T> > tmp(new Handle<T>(*this,std::unique_ptr<T>(obj)));
			auto &ret = *tmp;
			place_correctly(std::move(tmp));
			return ret;
		}

		template<typename T>
		Handle<T>& newHandle(T* r){
			std::unique_ptr<Handle<T> > tmp(new Handle<T>(*this,std::unique_ptr<T>(r)));
			auto &ret = *tmp;
			place_correctly(std::move(tmp));
			return ret;
		}

		template<typename T>
		T& get(Handle<T> &hndl) {return hndl;}

		template<typename T>
		void give(Handle<T> &hndl, std::unique_ptr<T> obj) {hndl = std::move(obj);}

		template<typename T>
		void give(Handle<T> &hndl, T* obj) {hndl = std::unique_ptr<T>(obj);}

		
		template<typename T>
		std::unique_ptr<T> take(Handle<T>& hndl){ return hndl;}

		template<typename T>
		std::unique_ptr<T> del(Handle<T>& hndl){
			std::unique_ptr<T> ret = hndl;
			assert(hndls[hndl.id]->id == hndl.id);
			hndls[hndl.id].reset(nullptr);
			return ret;
		}

		//commutative operations

		template<typename T>
		void incr_op(Handle<T> &h) {h.stored_obj->operator++();}

		template<typename T>
		void incr(Handle<T> &h) {h.stored_obj->incr();}

		template<typename T, typename... A>
		void add(Handle<T> &h, A... args) {h.stored_obj->add(args...);}

		template<typename T, typename F, typename... A>
		void add_f(Handle<T> &h, F addfun, A... args) {F(*(h.stored_obj), args...);}



		int return_one () {return 1;}
		DataStore () {}
		DataStore (const DataStore<L> &) = delete;
		virtual ~DataStore() {
			this-> destructing = true;
		}
		
		friend class HandlePrime;
	};


};
