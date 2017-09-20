//new handle.  the old one is pretty broken.
#pragma once
#include "Basics.hpp"
#include "RemoteObject.hpp"
#include "tracker/Tracker.hpp"
#include "mtl/top.hpp"
#include "mtl/mtlutils.hpp"
#include <memory>

namespace myria{


  namespace tracker{
    class Tracker;
  }
  
  template<typename,typename>
  struct Operation;

  template<typename label>
  struct GenericHandle;

  template<typename label>
  struct GenericHandle<Label<label> > {virtual ~GenericHandle() = default;};

  template<typename T>
  struct LabelFreeHandle {
  protected:
    virtual std::shared_ptr<const T> get(mtl::GPhaseContext *tc) const  = 0;
    virtual void put(mtl::GPhaseContext *tc, const T& t) = 0;
  public:
    virtual ~LabelFreeHandle() = default;
    friend class tracker::Tracker;
  };

  template<typename l, typename T, typename... SupportedOperations>
	std::unique_ptr<Handle<l,T,SupportedOperations...> > make_unmatched(char const * const v, std::size_t size);
	
  template<typename l, typename T, typename... SupportedOperations>
  struct Handle : public mutils::ByteRepresentable, public GenericHandle<l>, public LabelFreeHandle<T>, public SupportedOperations::template SupportsOn<Handle<l,T,SupportedOperations...> >... {

	  template<typename SO>
		  using OperationSuperclass = typename SO::template SupportsOn<Handle>;
	  
    std::shared_ptr<RemoteObject<l,T> > _ro;
  private:
    //for dropping operation support
    //the first parameter is to ensure we are calling this constructor intentionally,
    //in the case where we really are ditching operations.  I'd like to disable it statically,
    //but enable_if doesn't play nicely with constructors and specializing this class wastes a ton
    //of space
    Handle(std::integral_constant<std::size_t, sizeof...(SupportedOperations)>*,decltype(_ro) _ro):_ro(_ro){

	}
  public:
    using label = l;
	using type = T;

    template<typename DataStore, template<typename> class RO>
      Handle(std::shared_ptr<RO<T> > _ro, DataStore& ds):
      SupportedOperations::template SupportsOn<Handle>
      (SupportedOperations::template SupportsOn<Handle>::template wrap_operation<RO>(ds))...,
      _ro(_ro){
	static_assert(std::is_same<typename DataStore::label,label>::value);
      }

    Handle& downCast() { return *this;}
		
		template<typename U>
		auto& upCast(const U& t){
			using target = DECT(*mutils::find_match<DECT(OperationSuperclass<SupportedOperations>::template ifMatches<U>())...>());
			return target::type::upCast(t);
		}
	
    int uid = mutils::gensym();
	
    RemoteObject<l,T>& remote_object() {
      assert(_ro);
      return *_ro;
    }

	template<template<typename> class RO, typename DataStore>
		Handle(mutils::identity_struct1<RO>, DataStore &ds):OperationSuperclass<SupportedOperations>(OperationSuperclass<SupportedOperations>::template wrap_operation<RO>(ds))... {
	}
	Handle(const Handle& h):OperationSuperclass<SupportedOperations>(h)...,_ro(h._ro) {
		
	}
	Handle& operator=(const Handle& o) {
		(OperationSuperclass<SupportedOperations>::operator=(o),...);
		_ro = o._ro;
		return *this;
	}

    typedef T stored_type;

		std::size_t to_bytes(char* v) const {
      //for serialization
      if (_ro) {
				auto accum = mutils::to_bytes_v(v,true,_ro->inherit_bytes_size());
				auto ret = accum + _ro->inherit_to_bytes(v + accum);
				whendebug(auto bsh = bytes_size());
				assert(ret == bsh);
				return ret;
      }
      else {
				((bool*)v)[0] = false;
				return sizeof(bool);
      }
    }

		std::size_t bytes_size() const {
      return sizeof(bool) + (_ro ? _ro->inherit_bytes_size() + sizeof(std::size_t) : 0);
    }

    static std::unique_ptr<Handle> from_bytes(mutils::DeserializationManager* rdc, char const *v){
			static_assert(sizeof(bool) == sizeof(char));
			static_assert(sizeof(bool) == 1);
			bool b = v[0];
			assert(b && "looks like we need to allow null handles after all");
			std::size_t size = ((std::size_t*) (v + 1))[0];
auto ret_ro = mutils::inherit_from_bytes<RemoteObject<l,T> >(rdc, v + sizeof(bool) + sizeof(std::size_t) );
			if (!ret_ro) {
				return make_unmatched<l,T,SupportedOperations...>(v, size);
			}
			else {
				auto ret = std::unique_ptr<Handle>{dynamic_cast<Handle*>(ret_ro.release()->wrapInHandle().release())};
				assert(ret);
				return ret;
			}
    }

    std::shared_ptr<const T> get(mtl::PhaseContext<l> *tc) const {
      assert(_ro);
      assert(tc);
      auto &ctx = *tc;
      
      //If the Transacion Context does not yet exist for this store, we create it now.
      auto &store_ctx = ctx.store_context(this->store() whendebug(, "calling get() via handle"));
			return _ro->get(&store_ctx);
    }
		Handle create_new(mtl::PhaseContext<l> *tc, const T& newt) const {
      assert(_ro);
      assert(tc);
      auto &ctx = *tc;
      
      //If the Transacion Context does not yet exist for this store, we create it now.
      auto &store_ctx = ctx.store_context(this->store() whendebug(, "calling get() via handle"));
			auto copy_of_this = *this;
			copy_of_this._ro = _ro->create_new(&store_ctx,newt);
			return copy_of_this;
    }
  protected:
    std::shared_ptr<const T> get(mtl::GPhaseContext *tc) const {
      auto *ctx = dynamic_cast<mtl::PhaseContext<l>*>(tc);
      assert(ctx);
      return get(ctx);
    }
  public:
    
    Handle clone() const {
      return *this;
    }

		Handle nulled() const {
			auto copy_of_this = *this;
			copy_of_this._ro = nullptr;
			return copy_of_this;
		}
    
    operator Handle<l,T>(){
      return Handle<l,T>((std::integral_constant<std::size_t,0>*)nullptr, _ro);
    }
    
    void put(mtl::PhaseContext<l> *tc, const T& t){
      assert(tc);
      auto &ctx = *tc;
			assert(_ro);
			return _ro->put(&ctx.store_context(this->store() whendebug(, "calling put() via handle")),t);
    }

  protected:
    void put(mtl::GPhaseContext *tc, const T& t){
      auto *ctx = dynamic_cast<mtl::PhaseContext<l>* >(tc);
      assert(ctx);
      return put(ctx,t);
    }
  public:

		auto timestamp() const {
			assert(_ro);
			return _ro->timestamp();
		}
    
    bool isValid(mtl::PhaseContext<l> *ctx) const {
      if (!_ro) return false;
      assert(ctx);
      auto *ptr = &ctx->store_context(this->store() whendebug(, "calling isValid via handle"));
      return _ro->isValid(ptr);
    }
    
    DataStore<l>& store() const {
      assert(dynamic_cast<DataStore<l>*>(&_ro->store()));
      return (DataStore<l>&) _ro->store();
    }
    
    auto name() const {
		assert(_ro);
		assert(((std::size_t)_ro.get()) > 10);
      return _ro->name();
    }
    
    bool operator<(const Handle& h) const {
      return _ro->id < h._ro->id;
    }
       
    template<typename l2, typename T2, typename... SupportedOperations2>
      friend struct Handle;
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

	template<typename l, typename T, typename... ops> struct typename_str<myria::Handle<l,T,ops...> > {
		static std::string f(){
			std::stringstream ss;
			ss << "Handle<" << l{} << "," << typename_str<T>::f() << ">";
			return ss.str();
		}
	};
  
  template<typename l, typename T,typename... Ops>
  void post_object(const std::function<void (char const *const, std::size_t)>&f,
									 const myria::Handle<l,T,Ops...>& h){
    auto size = ::mutils::bytes_size(h);
    char buf[size];
    h.to_bytes(buf);
    f(buf,size);
  }

#ifndef NDEBUG
  template<typename l, typename T, typename... Ops>
  void ensure_registered(const myria::Handle<l,T,Ops...>& v, DeserializationManager& dm){
    ensure_registered(*v._ro,dm);
  }
#endif
  
	template<typename T, typename P>
  std::enable_if_t<myria::is_handle<T>::value,mutils::context_ptr<T> > from_bytes_noalloc(P* p, char const *v, context_ptr<T> = context_ptr<T>{}){
    return mutils::context_ptr<T>{T::from_bytes(p,v).release()};
  }
}
