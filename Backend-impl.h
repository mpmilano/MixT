#pragma once

private:

		class HandlePrime;
		template<typename T>
		class HandleImpl;

		std::map<uint, std::unique_ptr<HandlePrime> > hndls;
		int get_next_id(){
			static int id = 0;
			return ++id;
		}

		int get_next_rid(){
			static int rinit = 0;
			return ++rinit;
		}

		class HandlePrime {
		private: 
			DataStore& parent;
			virtual bool is_abstract() = 0;


			HandlePrime(DataStore& newParent, const HandlePrime &old):
				parent(newParent),
				id(old.id),
				rid(old.rid){assert(&old.parent != &parent);}

			HandlePrime(DataStore& parent):
				parent(parent),
				id(parent.get_next_id()),
				rid(parent.get_next_rid()){
				//most dangerous game
			}

		public:
			const uint id;
			const uint rid;

			HandlePrime(const HandlePrime &old) = delete;

			virtual std::unique_ptr<HandlePrime> clone (DataStore& np) const = 0;

			template<typename T>
			friend class HandleImpl;

		};

		void place_correctly(std::unique_ptr<HandlePrime> h){
			hndls[h->id] = std::move(h);
		}

		template<typename T> 
		class HandleImpl : public HandlePrime {

		public:
			std::unique_ptr<T> stored_obj;
		private:
			virtual bool is_abstract()  {return false;}
		public:
			operator T& (){ return *(this->stored_obj);}
			void operator =(std::unique_ptr<T> n) {this->stored_obj = std::move(n);}
			operator std::unique_ptr<T>() {return std::move(this->stored_obj);}

		private:
			HandleImpl(DataStore& parent,std::unique_ptr<T> n):HandlePrime(parent),stored_obj(std::move(n)){}
			HandleImpl(DataStore& parent,const HandleImpl& old):
				HandlePrime(parent,old),
				stored_obj(new T(*old.stored_obj)){}
		public: 
			friend class DataStore;
			template<Client_Id>
			friend class Client;
			HandleImpl (const HandleImpl&) = delete;

			static HandleImpl<T>& constructAndPlace(DataStore& parent,std::unique_ptr<T> n){
				HandleImpl<T>* hi = new HandleImpl<T>(parent, std::move(n));
				HandlePrime* h = hi;
				parent.hndls[h->id] = std::unique_ptr<HandlePrime>(h);
				return *hi;
			}

			static HandleImpl<T>& constructAndPlace(DataStore& parent,const HandleImpl& old) {
				HandleImpl<T>* hi = new HandleImpl<T>(parent, old);
				HandlePrime* h = hi;
				parent.hndls[h->id] = std::unique_ptr<HandlePrime>(h);
				return *hi;
			}

			virtual std::unique_ptr<HandlePrime> clone (DataStore& np) const {
				HandleImpl<T> const &h = *this;
				return std::unique_ptr<HandlePrime >(new HandleImpl<T>(np,h));
			}
			
			virtual ~HandleImpl() {}
			
		};

	public:

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
			place_correctly(std::move(tmp));
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


