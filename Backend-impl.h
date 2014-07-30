#pragma once

public:

template<typename T>
class HandleImpl;

private:


		class HandlePrime;
		typedef std::unique_ptr<HandlePrime> (*copy_h) (const HandlePrime&, DataStore&);

		class HandlePair{
		public:
			std::unique_ptr<HandlePrime> first;
			copy_h second;
			HandlePair():first(nullptr),second([](const HandlePrime&, DataStore&) {
					assert(false && "There's a bug!");
					return std::unique_ptr<DataStore::HandlePrime>(nullptr);}){}
			HandlePair(std::unique_ptr<HandlePrime> &&first, copy_h second):first(std::move(first)),second(second) {}

		};


		std::vector<HandlePair > hndls;
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

		static int get_next_rid(){
			static int rinit = 0;
			return ++rinit;
		}
		class HandlePrime {
		private: 
			DataStore& parent;
			virtual bool is_abstract() = 0;
		public:
			const uint id;
			const uint rid;
			HandlePrime(DataStore& parent):
				parent(parent),
				id(parent.get_next_id()),
				rid(get_next_rid()){}

			HandlePrime(DataStore& newParent, const HandlePrime &old):
				parent(newParent),
				id(old.id),
				rid(old.rid){}

			HandlePrime(const HandlePrime &old) = delete;

			virtual ~HandlePrime() {
				if (!parent.destructing){
					parent.next_ids.push(id);
				}
			}
		};

		void place_correctly(std::unique_ptr<HandlePrime> hndl, copy_h f){
			auto h = HandlePair(std::move(hndl), f);
			if (h.first->id == hndls.size()) {
				hndls.push_back(std::move(h));
			}
			else {
				assert (h.first->id < hndls.size());
				assert (! hndls[h.first->id].first);
				hndls[h.first->id] = std::move(h);
			}

		}


	public:

		template<typename T> 
		class HandleImpl : public HandlePrime {
		private:
			std::unique_ptr<T> stored_obj;
			virtual bool is_abstract()  {return false;}
			operator T& (){ return *(this->stored_obj);}
			void operator =(std::unique_ptr<T> n) {this->stored_obj = std::move(n);}
			operator std::unique_ptr<T>() {return std::move(this->stored_obj);}
			HandleImpl(DataStore& parent,std::unique_ptr<T> n):HandlePrime(parent),stored_obj(std::move(n)){}
			HandleImpl(DataStore& parent,const HandleImpl& old):
				HandlePrime(parent,old),
				stored_obj(new T(*old.stored_obj)){}
		public: 
			friend class DataStore;
			template<Client_Id>
			friend class Client;
			HandleImpl (const HandleImpl&) = delete;
			
			virtual ~HandleImpl() {}
			
		};

		class GenericHandle{
		private: 
			virtual bool is_virtual() = 0;
		};

		template <typename T>
		class TypedHandle : public GenericHandle{
		private: 
			virtual bool is_virtual() = 0;
			HandleImpl<T> &h_i;
			HandleImpl<T> &hi() const {return h_i;}
		public:
			TypedHandle(HandleImpl<T> &hi):h_i(hi){}
			friend class DataStore;
			template<Client_Id>
			friend class Client;
		};

		template<Client_Id id, Level L, HandleAccess HA, typename T>
		class Handle : public TypedHandle<T> {
		private:
			virtual bool is_virtual() {return false;}
		public: 
			Handle(HandleImpl<T> &hi):TypedHandle<T>(hi){}
			static constexpr Level level = L;
			static constexpr HandleAccess ha = HA;
			typedef T stored_type;
			friend class DataStore;
			template<Client_Id>
			friend class Client;
		};

		template<Client_Id id, Level L, HandleAccess HA, typename T>	
		auto newhandle_internal(std::unique_ptr<T> r) {
			std::unique_ptr<HandleImpl<T> > tmp(new HandleImpl<T>(*this,std::move(r)));
			auto &ret = *tmp;
			static const copy_h copyf = [](const HandlePrime &_hp, DataStore& np){
				auto *_h = static_cast<const HandleImpl<T>* >(&_hp);
				HandleImpl<T> const &h = *_h;
				return std::unique_ptr<HandlePrime >(new HandleImpl<T>(np,h));
			};
			place_correctly(std::move(tmp), copyf);
			return Handle<id,L,HA,T>(ret);
		}

		template<Client_Id id, Level L, HandleAccess HA, typename T>
		auto del_internal(Handle<id,L,HA,T> &hndl_i){
			auto &hndl = hndl_i.hi(); 
			std::unique_ptr<T> ret = hndl;
			assert(hndls[hndl.id]->id == hndl.id);
			hndls[hndl.id].first.reset(nullptr);
			return ret;
		}


