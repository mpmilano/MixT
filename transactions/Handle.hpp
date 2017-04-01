//new handle.  the old one is pretty broken.
#pragma once
#include "../BitSet.hpp"
#include "Basics.hpp"
#include "RemoteObject.hpp"
#include "Tracker.hpp"
#include "top.hpp"
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


    std::shared_ptr<RemoteObject<l,T> > _ro;
  private:
    //for dropping operation support
    //the first parameter is to ensure we are calling this constructor intentionally,
    //in the case where we really are ditching operations.  I'd like to disable it statically,
    //but enable_if doesn't play nicely with constructors and specializing this class wastes a ton
    //of space
    Handle(std::integral_constant<std::size_t, sizeof...(SupportedOperations)>*,decltype(_ro) _ro):_ro(_ro){}
  public:
    using label = l;
		using type = T;

    /**
     * use this constructor for *new* objects
     */
    template<typename DataStore, template<typename> class RO>
      Handle(tracker::Tracker &trk, mtl::PhaseContext<l> *tc, std::shared_ptr<RO<T> > _ro, DataStore& ds):
      SupportedOperations::template SupportsOn<Handle>(SupportedOperations::template SupportsOn<Handle>::template wrap_operation<RO>(ds))...,
      _ro(_ro){
				static_assert(std::is_same<typename DataStore::label,label>::value);
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
	static_assert(std::is_same<typename DataStore::label,label>::value);
      }

    Handle& downCast() { return *this;}
	
    int uid = mutils::gensym();
	
    RemoteObject<l,T>& remote_object() {
      assert(_ro);
      return *_ro;
    }

    Handle() {}
    Handle(const Handle& h) = default;
		Handle& operator=(const Handle& o) = default;

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

    std::shared_ptr<const T> get(mtl::PhaseContext<l> *tc) const {
      assert(_ro);
      assert(tc);
      auto &ctx = *tc;
      
      //If the Transacion Context does not yet exist for this store, we create it now.
      auto &store_ctx = ctx.store_context(this->store() whendebug(, "calling get() via handle"));

      constexpr std::integral_constant<bool, !l::requires_causal_tracking::value> *choice{nullptr};
      return get(choice,store_ctx, ctx.trackingContext, _ro->get(&store_ctx));
    }
    
    std::shared_ptr<const T> get(std::true_type*, mtl::StoreContext<l>& ctx, tracker::TrackingContext &, std::shared_ptr<const T> ret) const{
      return ret;
    }
    
    std::shared_ptr<const T> get(std::false_type*, mtl::StoreContext<l>& ctx, tracker::TrackingContext &, std::shared_ptr<const T> ret) const{
			return _ro->get(&ctx);
    }
    
    Handle clone() const {
      return *this;
    }
    
    operator Handle<l,T>(){
      return Handle<l,T>((std::integral_constant<std::size_t,0>*)nullptr, _ro);
    }
    
    void put(mtl::PhaseContext<l> *tc, const T& t){
      assert(tc);
      auto &ctx = *tc;
      constexpr std::integral_constant<bool, !l::requires_causal_tracking::value> *choice{nullptr};
      return put(ctx,t,choice);
    }
    
    void put(mtl::PhaseContext<l> &ctx, const T& t, std::true_type*) {
      assert(_ro);
			_ro->put(&ctx.store_context(this->store() whendebug(, "calling put() via handle")),t);
    }
    
    void put(mtl::PhaseContext<l> &ctx, const T& t, std::false_type*) {
      assert(_ro);
			_ro->put(&ctx.store_context(this->store() whendebug(, "calling put() via handle")),t);
    }
    
    bool isValid(mtl::PhaseContext<l> *ctx) const {
      if (!_ro) return false;
      assert(ctx);
      auto *ptr = &ctx->store_context(this->store() whendebug(, "calling isValid via handle"));
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
    static void do_onwrite(mtl::PhaseContext<l> &, tracker::Tracker &tr, RemoteObject<l,T> &ro,std::false_type*){
      tr.onStrongWrite(ro.store(),ro.name(),(T*)nullptr);
    }
    static void do_onwrite(mtl::PhaseContext<l> &, tracker::Tracker &tr, RemoteObject<l,T> &ro,std::true_type*){
      tr.onCausalWrite(ro.store(),ro.name(),ro.timestamp(),(T*)nullptr);
    }
		static void do_onwrite(mtl::PhaseContext<l> &ctx, tracker::Tracker &tr, RemoteObject<l,T> &ro){
			typename l::requires_causal_tracking* choice{nullptr};
			do_onwrite(ctx,tr,ro,choice);
		}
  };
  
  template<typename T>
  struct is_handle;
  template<typename l, typename T, typename... Ops>
  struct is_handle<Handle<l,T,Ops...> > : std::true_type {};
  template<typename T>
  struct is_handle : std::false_type {};

	template<typename T>
	struct label_from_handle_str;
	template<typename l, typename T, typename... Ops>
	struct label_from_handle_str<Handle<l,T,Ops...> >{
		using type = l;
	};
	template<typename T> using label_from_handle = typename label_from_handle_str<T>::type;
  
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
