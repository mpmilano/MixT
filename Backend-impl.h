public:

template<typename T>
class HandleImpl;

private: 
		class HandlePrime;
		std::vector<std::unique_ptr<HandlePrime> > hndls;
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
		class HandleImpl : public HandlePrime {
		private:
			std::unique_ptr<T> stored_obj;
			virtual bool is_abstract()  {return false;}
			operator T& (){ return *(this->stored_obj);}
			void operator =(std::unique_ptr<T> n) {this->stored_obj = std::move(n);}
			operator std::unique_ptr<T>() {return std::move(this->stored_obj);}
			HandleImpl(DataStore& parent,std::unique_ptr<T> n):HandlePrime(parent),stored_obj(std::move(n)){}
		public: 
			friend class DataStore;
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
		};

		template<Level L, HandleAccess HA, typename T>
		class Handle : public TypedHandle<T> {
		private:
			virtual bool is_virtual() {return false;}
		public: 
			Handle(HandleImpl<T> &hi):TypedHandle<T>(hi){}
			static constexpr Level level = L;
			static constexpr HandleAccess ha = HA;
			typedef T stored_type;
			friend class DataStore;
		};

		template<Level L, HandleAccess HA, typename T>	
		Handle<L, HA, T> newhandle_internal(std::unique_ptr<T> r) {
			std::unique_ptr<HandleImpl<T> > tmp(new HandleImpl<T>(*this,std::move(r)));
			auto &ret = *tmp;
			place_correctly(std::move(tmp));
			return Handle<L,HA,T>(ret);
		}

		template<Level L, HandleAccess HA, typename T>
		std::unique_ptr<T> del_internal(Handle<L,HA,T> &hndl_i){
			auto &hndl = hndl_i.hi(); 
			std::unique_ptr<T> ret = hndl;
			assert(hndls[hndl.id]->id == hndl.id);
			hndls[hndl.id].reset(nullptr);
			return ret;
		}

template<bool b>
struct neg : std::integral_constant<bool, !b> {};

template <typename C>
static constexpr std::integral_constant<bool,true> is_not_handle_f(C*);

template < Level L, HandleAccess HA, typename T>
static constexpr std::integral_constant<bool,false> is_not_handle_f(DataStore::Handle<L,HA,T>*);

template<typename T>
struct is_not_handle : decltype( is_not_handle_f ( (T*) nullptr) ) {};

template<typename T>
struct is_handle : neg<is_not_handle<T>::value> {};


template <typename C>
static constexpr std::integral_constant<bool,true> handle_no_read_f(C*);

template < Level L, HandleAccess HA, typename T>
	static constexpr std::integral_constant<bool,!canRead(HA)> handle_no_read_f(DataStore::Handle<L,HA,T>*);

template<typename T>
struct handle_no_read : decltype( handle_no_read_f ( (T*) nullptr) ) {};

template<typename T>
struct handle_read : neg<handle_no_read<T>::value> {};


template <typename C>
static constexpr std::integral_constant<bool,true> handle_no_write_f(C*);

template < Level L, HandleAccess HA, typename T>
	static constexpr std::integral_constant<bool,!canWrite(HA)> handle_no_write_f(DataStore::Handle<L,HA,T>*);

template<typename T>
struct handle_no_write : decltype( handle_no_write_f ( (T*) nullptr) ) {};

template<typename T>
struct handle_write : neg<handle_no_write<T>::value> {};

template<typename... Args>
struct all_handles : bool_const<! any<is_not_handle, pack<Args...> >::value > {};

template<typename... Args>
struct all_handles_read : bool_const<! any <handle_no_read, pack<Args...> >::value > {};

template<typename... Args>
struct all_handles_write : bool_const <! any <handle_no_write, pack<Args...> >::value> {};

template<typename... Args>
struct exists_write_handle : any <handle_write, pack<Args...> > {};

template<typename... Args>
struct exists_read_handle : any <handle_read, pack<Args...> > {};
