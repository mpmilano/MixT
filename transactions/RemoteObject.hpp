#pragma once

#include "type_utils.hpp"
#include "tuple_extras.hpp"
#include "SerializationSupport.hpp"
#include "DataStore.hpp"

namespace myria{
  namespace tracker {
    class Tracker;
  }


  struct GeneralRemoteObject{
    const int id = mutils::gensym();
    virtual const GDataStore& store() const = 0;
    virtual GDataStore& store() = 0;
    virtual Name name() const = 0;
    virtual const std::array<int,NUM_CAUSAL_GROUPS>& timestamp() const = 0;
    virtual ~GeneralRemoteObject() = default;
  };

  template<typename T>
  struct TypedRemoteObject : public GeneralRemoteObject{
    virtual ~TypedRemoteObject() = default;
  };

  template<typename l2, typename T2,typename...> struct Handle;
  
  template<typename l, typename T>
  class RemoteObject : public TypedRemoteObject<T>
  {
    //extend this plz!
		using level = l;
    
    virtual bool ro_isValid(mtl::StoreContext<l>*) const = 0;
    virtual std::shared_ptr<const T> get(mtl::StoreContext<l>*, tracker::Tracker*/* = nullptr*/, tracker::TrackingContext*/* = nullptr*/) = 0;
    virtual void put(mtl::StoreContext<l>*,const T&) = 0;
    
    //TODO: delete these when you're done hacking around.
    RemoteObject(const RemoteObject&) = delete;
    
  public:

		virtual const DataStore<level>& store() const = 0;
    virtual DataStore<level>& store() = 0;
		
    RemoteObject(){}
    virtual ~RemoteObject() = default;
    template<typename l2, typename T2,typename...>
    friend struct Handle;
    
    using type = T;
    std::vector<char> o_bytes(mtl::StoreContext<l>* sc, tracker::Tracker* trk, tracker::TrackingContext* tc) {
      std::vector<char> ret;
      auto retT = get(sc,trk,tc);
      ret.resize(mutils::bytes_size(*retT));
      mutils::to_bytes(*retT,ret.data());
      assert(ret.data());
      assert(ret.size() > 0);
      return ret;
    }
    
    friend class tracker::Tracker;
    
  };

  template<typename> struct is_RemoteObject;
  template<typename l, typename T> struct is_RemoteObject<RemoteObject<l,T> > : std::true_type {};
  
  template<typename T>
  static std::unique_ptr<mutils::type_check<is_RemoteObject,T> > from_bytes(mutils::DeserializationManager*, char const * ){
    static_assert(!is_RemoteObject<T>::value,"Error: Do not directly attempt to deserialize a Remote object.  It is not safe.  Deserialize the handle.");
  }

}

