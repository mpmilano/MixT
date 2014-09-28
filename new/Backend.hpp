
enum class Level {weak, strong};

//handles and accompanying utilities.
//maybe needs a namespace.

namespace Access{
enum class Access {read, write, both, admin};

	constexpr bool read(Access a){
		return a != write;
	}

	constexpr bool write(Access a){
		return a != read;
	}

	constexpr bool noread(Access a){
		return a == write;
	}

	constexpr bool nowrite(Access a){
		return a == read;
	}

}

typedef int location;

template<location l>
class Instance{

	enum class LSWhen {deferred, immediate};
	template<LSWhen when>
	class LogStore{

		template<typename Args...>
		void add(std::function<void (Args...)> f, Args... params){
			static_assert();
			//TODO body here
		}
	};

	class StoredBlob  {
	private:
		virtual bool is_abstract() = 0;
		
		static int get_next_id(){
			static int id = 0;
			return ++id;
		}
		StoredBlob(DataStore& newParent, const HandlePrime &old):
			parent(newParent),
			id(old.id){assert(&old.parent != &parent);}
		
		HandlePrime(DataStore& parent):
			parent(parent),
			id(get_next_id()),
			rid(get_next_rid()){
		}
		
	public:
		
		DataStore& parent;
		const uint id;
		
		HandlePrime(const HandlePrime &old) = delete;
	
		virtual ~HandlePrime() {}
		
		virtual void grab_obj(const HandlePrime &) = 0;
	
		template<typename T>
		friend class DataStore::HandleImpl;
	
	}

	template<typename T> 
	class StoredObject {
		std::unique_ptr<T> t;
		
	};

	template<typename T, Level l, Access::Access a>
	class Handle{
		
	};
};
