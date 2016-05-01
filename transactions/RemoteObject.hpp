#pragma once

#include "type_utils.hpp"
#include "tuple_extras.hpp"
#include "GDataStore.hpp"
#include "Store.hpp"
#include "SerializationSupport.hpp"

namespace myria{

	namespace mtl {
		template<typename> struct Transaction;
	}

	namespace tracker {
		class Tracker;
	}

#define GeneralRemoteObject_body										\
	const int id = mutils::gensym();									\
	virtual const DataStore<level>& store() const = 0;					\
	virtual DataStore<level>& store() = 0;								\
	virtual Name name() const = 0;										\
	virtual ~GeneralRemoteObject(){}									\

	template<Level>
	struct GeneralRemoteObject;

	template<>
	struct GeneralRemoteObject<Level::strong> : public mutils::ByteRepresentable{
		static constexpr Level level = Level::strong;
		GeneralRemoteObject_body
	};

	template<>
	struct GeneralRemoteObject<Level::causal> : public mutils::ByteRepresentable{
		static constexpr Level level = Level::causal;
		GeneralRemoteObject_body
		virtual const std::array<int,NUM_CAUSAL_GROUPS>& timestamp() const = 0;
	};


	template<Level l2, HandleAccess ha2, typename T2,typename...> struct Handle;

	template<Level l, typename T>
	class RemoteObject : public GeneralRemoteObject<l>
	{
		//extend this plz!

		virtual bool ro_isValid(mtl::StoreContext<l>*) const = 0;
		virtual std::shared_ptr<const T> get(mtl::StoreContext<l>*, tracker::Tracker*/* = nullptr*/, tracker::TrackingContext*/* = nullptr*/) = 0;
		virtual void put(mtl::StoreContext<l>*,const T&) = 0;

		//TODO: delete these when you're done hacking around.
		RemoteObject(const RemoteObject&) = delete;

	public:
		RemoteObject(){}
		virtual ~RemoteObject(){}
		template<Level l2, HandleAccess HA, typename T2,typename...>
		friend struct Handle;

		using type = T;
	
		/*
		std::vector<char> bytes() const {
			assert(false && "ERROR! you're using this when you wanted the *stored* type,"
				   && "but this thing returns the pointer as bytes!  this is a problem!"
				&& "incidentally, this also means that the TrackerTestingStore has a problem with"
				&& "tobytes, so you should fix that too");
			std::vector<char> ret;
			ret.resize(this->bytes_size());
			this->to_bytes(ret.data());
			assert(ret.size() > 0);
			return ret;
			}//*/

		std::vector<char> o_bytes(mtl::StoreContext<l>* sc, tracker::Tracker* trk, tracker::TrackingContext* tc) {
			std::vector<char> ret;
			auto retT = get(sc,trk,tc);
			ret.resize(mutils::bytes_size(*retT));
			mutils::to_bytes(*retT,ret.data());
			assert([&](){
					if (ret.size() == 0){
						std::cout << *retT << std::endl;
						std::cout << mutils::type_name<T>() << std::endl;
						std::cout << mutils::bytes_size(*retT) << std::endl;
					}
					return true;}());
			assert(ret.data());
			assert(ret.size() > 0);
			return ret;
		}

		template<typename> friend struct mtl::Transaction;
		friend class tracker::Tracker;

	};

	template<typename> struct is_RemoteObject;
	template<Level l, typename T> struct is_RemoteObject<RemoteObject<l,T> > : std::true_type {};

	template<typename T>
	static std::unique_ptr<mutils::type_check<is_RemoteObject,T> > from_bytes(mutils::DeserializationManager*, char const * ){
		static_assert(!is_RemoteObject<T>::value,"Error: Do not directly attempt to deserialize a Remote object.  It is not safe.  Deserialize the handle.");
	}

}

