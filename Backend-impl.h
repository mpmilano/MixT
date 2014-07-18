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

		template<Level L, typename T>
		class Handle {
		private:
			HandleImpl<T> &hi;
			Handle(HandleImpl<T> &hi):hi(hi){}
		public: 
			static constexpr Level level = L;
			friend class DataStore;
			virtual ~Handle() {}

		};

		template<Level L, typename T>	
		Handle<L, T> newhandle_internal(std::unique_ptr<T> r) {
			std::unique_ptr<HandleImpl<T> > tmp(new HandleImpl<T>(*this,std::move(r)));
			auto &ret = *tmp;
			place_correctly(std::move(tmp));
			return Handle<L,T>(ret);
		}

		template<Level L, typename T>
		std::unique_ptr<T> del_internal(Handle<L, T> &hndl_i){
			auto &hndl = hndl_i.hi; 
			std::unique_ptr<T> ret = hndl;
			assert(hndls[hndl.id]->id == hndl.id);
			hndls[hndl.id].reset(nullptr);
			return ret;
		}

