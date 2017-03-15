//new handle.  the old one is pretty broken.
#pragma once
#include "../BitSet.hpp"
#include "Basics.hpp"
#include "RemoteObject.hpp"
#include "Store.hpp"
#include "Tracker.hpp"
#include <memory>

namespace myria{


  template<typename,typename>
  struct Operation;

  template<typename label>
  struct GenericHandle;

  template<typename label>
  struct GenericHandle<Label<label> > {virtual ~GenericHandle() = default;};

  template<typename T>
  struct LabelFreeHandle {virtual ~LabelFreeHandle() = default;};

  template<typename l2, typename T2, typename... ops2>
  std::unique_ptr<Handle<l2,ops2...> > hndl_from_bytes(mutils::DeserializationManager* dm, char const * __v);
	
  template<typename l, typename T, typename... SupportedOperations>
  struct Handle : public GenericHandle<l>, public LabelFreeHandle<T>, public SupportedOperations::template SupportsOn<Handle<l,T,SupportedOperations...> >... {


    const std::shared_ptr<RemoteObject<l,T> > _ro;
  private:
    //for dropping operation support
    //the first parameter is to ensure we are calling this constructor intentionally,
    //in the case where we really are ditching operations.  I'd like to disable it statically,
    //but enable_if doesn't play nicely with constructors and specializing this class wastes a ton
    //of space
    Handle(std::integral_constant<std::size_t, sizeof...(SupportedOperations)>*,decltype(_ro) _ro):_ro(_ro){}
  public:

    using level_t = label;
    using label = l;

    /**
     * use this constructor for *new* objects
     */
    template<typename DataStore, template<typename> class RO>
      Handle(tracker::Tracker &trk, mtl::TransactionContext *tc, std::shared_ptr<RO<T> > _ro, DataStore& ds):
      SupportedOperations::template SupportsOn<Handle>(SupportedOperations::template SupportsOn<Handle>::template wrap_operation<RO>(ds))...,
      _ro(_ro){
	static_assert(std::is_same<DataStore::level,label>::value);
	assert(tc);
	auto &ctx = *tc;
	do_onwrite(ctx,trk,*_ro);
      }

    /**
     * use this constructor for *existing* objects
     */
    template<typename DataStore, template<typename> class RO>
      Handle(std::shared_ptr<RO<T> > _ro, DataStore& ds):
      SupportedOperations::template SupportsOn<Handle>
      (SupportedOperations::template SupportsOn<Handle>::template wrap_operation<RO>(ds))...,
      _ro(_ro){
	static_assert(std::is_same<DataStore::level,label>::value);
      }

    Handle& downCast() { return *this;}
	
    const int uid = mutils::gensym();
	
    typename RemoteObject<l,T>& remote_object() {
      assert(_ro);
      return *_ro;
    }

    Handle() {}
    Handle(const Handle& h) = default;
		
    using level = level_t;

    typedef T stored_type;

    int to_bytes_hndl(char* v) const {
      //for serialization
      if (_ro) {
	((bool*)v)[0] = true;
	return sizeof(bool) + _ro->to_bytes(v + sizeof(bool));
      }
      else {
	((bool*)v)[0] = false;
	return sizeof(bool);
      }
    }

    int bytes_size_hndl() const {
      return sizeof(bool) + (_ro ? _ro->bytes_size() : 0);
    }

    static std::unique_ptr<Handle> from_bytes(mutils::DeserializationManager* rdc, char const *v){
      return hndl_from_bytes<l,T,SupportedOperations...>(rdc,v);
    }

    std::shared_ptr<const T> get(tracker::Tracker& tracker, mtl::TransactionContext *tc) const {
      assert(_ro);
      assert(tc);
      auto &ctx = *tc;
      assert(ctx.trackingContext);
      
      //If the Transacion Context does not yet exist for this store, we create it now.
      auto &store_ctx = *ctx.template get_store_context<l>(_ro->store() whendebug(,"calling get() via handle"));

      constexpr std::integral_constant<bool, !l::requires_causal_tracking()> *choice{nullptr};
      return get(choice,tracker,store_ctx, *ctx.trackingContext, _ro->get(&store_ctx,&tracker,ctx.trackingContext.get()));
    }
    
    std::shared_ptr<const T> get(std::true_type*, tracker::Tracker& tracker, mtl::StoreContext<l>& ctx, tracker::TrackingContext &trkc, std::shared_ptr<const T> ret) const{
      tracker.afterStrongRead(ctx,trkc,_ro->store(),_ro->name(),(T*)nullptr);
      return ret;
    }
    
    std::shared_ptr<const T> get(std::false_type*, tracker::Tracker& tracker, mtl::StoreContext<l>& ctx, tracker::TrackingContext &trkc, std::shared_ptr<const T> ret)const{
      mutils::AtScopeEnd ase{[&](){tracker.afterCausalRead(trkc,_ro->store(),_ro->name(),_ro->timestamp(),_ro->o_bytes(&ctx,&tracker,&trkc),(T*)nullptr);}};
      if (tracker.waitForCausalRead(trkc,_ro->store(),_ro->name(),_ro->timestamp(),(T*)nullptr)){
	return ret;
      }
      else return _ro->get(&ctx,&tracker,&trkc);
    }
    
    Handle clone() const {
      return *this;
    }
    
    operator Handle<l,ha,T>(){
      return Handle<l,ha,T>((std::integral_constant<std::size_t,0>*)nullptr, _ro);
    }
    
    void put(tracker::Tracker& tracker, mtl::TransactionContext *tc, const T& t){
      assert(tc);
      auto &ctx = *tc;
      assert(ctx.trackingContext);
      constexpr std::integral_constant<bool, !l::requires_causal_tracking()> *choice{nullptr};
      return put(tracker, ctx,t,choice);
    }
    
    void put(tracker::Tracker& tracker, mtl::TransactionContext &ctx, const T& t, std::true_type*) {
      assert(_ro);
      tracker.onStrongWrite(ctx,_ro->store(),_ro->name(),(T*)nullptr);
      _ro->put(ctx.template get_store_context<l>(_ro->store(),"calling put() via handle").get(),t);
    }
    
    void put(tracker::Tracker& tracker, mtl::TransactionContext &ctx, const T& t, std::false_type*) {
      assert(_ro);
      tracker.onCausalWrite(_ro->store(),_ro->name(),_ro->timestamp(),(T*)nullptr);
      _ro->put(ctx.template get_store_context<l>(_ro->store(),"calling put() via handle").get(),t);
    }
    
    bool isValid(mtl::TransactionContext *ctx) const {
      if (!_ro) return false;
      assert(ctx);
      auto *ptr = ctx->template get_store_context<l>(_ro->store(),"calling isValid via handle").get();
      return _ro->ro_isValid(ptr);
    }
    
    DataStore<l>& store() const {
      assert(dynamic_cast<DataStore<l>*>(&_ro->store()));
      return (DataStore<l>&) _ro->store();
    }
    
    auto name() const {
      return _ro->name();
    }
    
    bool operator<(const Handle& h) const {
      return _ro->id < h._ro->id;
    }
       
    template<typename l2, typename T2, typename... SupportedOperations2>
      friend struct Handle;
    
  private:
    static void do_onwrite(mtl::TransactionContext &tc, tracker::Tracker &tr, RemoteObject<l,T> &ro, std::enable_if_t<!l::requires_causal_tracking()>* = nullptr){
      tr.onWrite(tc,ro.store(),ro.name(),(T*)nullptr);
    }
    static void do_onwrite(mtl::TransactionContext &, tracker::Tracker &tr, RemoteObject<Level::causal,T> &ro, std::enable_if_t<l::requires_causal_tracking()>* = nullptr){
      tr.onWrite(ro.store(),ro.name(),ro.timestamp(),(T*)nullptr);
    }
  };
  
  template<typename T>
  struct is_handle;
  template<Level l, HandleAccess ha, typename T, typename... Ops>
  struct is_handle<Handle<l,ha,T,Ops...> > : std::true_type {};
  template<typename T>
  struct is_handle : std::false_type {};
  
  template<typename T>
  struct is_not_handle : std::integral_constant<bool, !is_handle<T>::value >::type {};
  
}
namespace mutils{
  
  template<typename l, typename T,typename... Ops>
  int to_bytes(const myria::Handle<l,T,Ops...>& h, char* v){
    return h.to_bytes_hndl(v);
  }
  
  
  template<typename l, typename T,typename... Ops>
  int bytes_size(const myria::Handle<l,T,Ops...> &h){
    return h.bytes_size_hndl();
  }
  
  template<typename l, typename T,typename... Ops>
  void post_object(const std::function<void (char const *const, std::size_t)>&f,
		   const myria::Handle<l,T,Ops...>& h){
    auto size = ::mutils::bytes_size(h);
    char buf[size];
    h.to_bytes_hndl(buf);
    f(buf,size);
  }
  
  template<typename l, typename T, typename... Ops>
  void ensure_registered(const myria::Handle<l,T,Ops...>& v, DeserializationManager& dm){
    ensure_registered(*v._ro,dm);
  }
  
  template<typename T, typename P>
  std::enable_if_t<myria::is_handle<T>::value,std::unique_ptr<T> > from_bytes(P* p, char const *v){
    return T::from_bytes(p,v);
  }
}
