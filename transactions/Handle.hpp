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


	//todo: move this


	namespace mtl {
	  
		template<typename >
		struct extract_type;
		
		template<typename>
		struct get_level;

		template<typename> struct Transaction;

		template<HandleAccess ha2, typename T2, typename... Ops>
		Handle<Level::strong,ha2,T2,Ops...> run_ast_causal(
			TransactionContext *ctx, const mtl::CausalCache& cache, const mtl::CausalStore &, const Handle<Level::strong,ha2,T2,Ops...>& h);
		
		template<HandleAccess ha, typename T2, typename... Ops>
		Handle<Level::strong,ha,T2,Ops...>
		run_ast_causal(TransactionContext *ctx, mtl::CausalCache& cache, const mtl::CausalStore &s,
					   const Handle<Level::strong,ha,T2,Ops...>& h);


	}

	template<Level l, HandleAccess HA>
	struct GenericHandle {};

	template<unsigned long long id, Level l, HandleAccess ha>
	std::nullptr_t find_usage(const GenericHandle<l,ha>&){
		return nullptr;
	}

	template<Level l2, HandleAccess ha2, typename T2, typename... ops2>
	std::unique_ptr<Handle<l2,ha2,T2,ops2...> > hndl_from_bytes(mutils::DeserializationManager* dm, char const * __v);
	
	template<Level l, HandleAccess HA, typename T, typename... SupportedOperations>
	struct Handle : public GenericHandle<l,HA>,
					public SupportedOperations::template SupportsOn<Handle<l,HA,T,SupportedOperations...> >... {


		const std::shared_ptr<RemoteObject<l,T> > _ro;
	private:
		//for dropping operation support
		//the first parameter is to ensure we are calling this constructor intentionally,
		//in the case where we really are ditching operations.  I'd like to disable it statically,
		//but enable_if doesn't play nicely with constructors and specializing this class wastes a ton
		//of space
		Handle(std::integral_constant<std::size_t, sizeof...(SupportedOperations)>*,decltype(_ro) _ro):_ro(_ro){}
	public:

		using level_t = std::integral_constant<Level,l>;

        /**
         * use this constructor for *new* objects
         */
		template<typename DataStore, template<typename> class RO>
			Handle(tracker::Tracker &trk, mtl::TransactionContext *tc, std::shared_ptr<RO<T> > _ro, DataStore& ds):
			SupportedOperations::template SupportsOn<Handle>(
				SupportedOperations::template SupportsOn<Handle>::template wrap_operation<RO>(ds))...,
			_ro(_ro){
			assert(_ro->store().level == l);
			assert(tc);
			auto &ctx = *tc;
			do_onwrite(ctx,trk,*_ro);
		}

            /**
             * use this constructor for *existing* objects
             */
            template<typename DataStore, template<typename> class RO>
                Handle(std::shared_ptr<RO<T> > _ro, DataStore& ds):
                SupportedOperations::template SupportsOn<Handle>(
                    SupportedOperations::template SupportsOn<Handle>::template wrap_operation<RO>(ds))...,
                _ro(_ro){
                assert(_ro->store().level == l);
            }


		Handle& downCast() { return *this;}
	
		const int uid = mutils::gensym();
	
		typename std::conditional<canWrite<HA>::value,
								  RemoteObject<l,T>&,
								  const RemoteObject<l,T>& >::type
		remote_object() const {
			assert(_ro);
			return *_ro;
		}

		Handle() {}
		Handle(const Handle& h) = default;
		
		static constexpr Level level = l;
		static constexpr HandleAccess ha = HA;
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
			return hndl_from_bytes<l,HA,T,SupportedOperations...>(rdc,v);
		}

		std::shared_ptr<const T> get(tracker::Tracker& tracker, mtl::TransactionContext *tc) const {
			assert(_ro);
			choose_strong<l> choice {nullptr};
			assert(_ro->store().level == l);
			/*
			assert(tc || !_ro->store().in_transaction());
			auto owner = (tc ? nullptr : std::make_unique<mtl::TransactionContext>((void*)nullptr,_ro->store().begin_transaction(),tracker.generateContext()));
			if (owner) owner->commit_on_delete = true;*/
			assert(tc);
			//auto &ctx = (owner ? *owner : *tc);
			auto &ctx = *tc;
			assert(ctx.trackingContext);

			//If the Transacion Context does not yet exist for this store, we create it now.
			auto &store_ctx = *ctx.template get_store_context<l>(_ro->store(),"calling get() via handle");

			return get(choice,tracker,store_ctx, *ctx.trackingContext, _ro->get(&store_ctx,&tracker,ctx.trackingContext.get()));
		}
	
		std::shared_ptr<const T> get(std::true_type*, tracker::Tracker& tracker, mtl::StoreContext<l>& ctx, tracker::TrackingContext &trkc, std::shared_ptr<const T> ret) const{
			tracker.afterRead(ctx,trkc,_ro->store(),_ro->name(),(T*)nullptr);
			return ret;
		}
	
		std::shared_ptr<const T> get(std::false_type*, tracker::Tracker& tracker, mtl::StoreContext<l>& ctx, tracker::TrackingContext &trkc, std::shared_ptr<const T> ret)const{
			mutils::AtScopeEnd ase{[&](){tracker.afterRead(trkc,_ro->store(),_ro->name(),_ro->timestamp(),_ro->o_bytes(&ctx,&tracker,&trkc),(T*)nullptr);}};
			if (tracker.waitForRead(trkc,_ro->store(),_ro->name(),_ro->timestamp(),(T*)nullptr)){
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
			choose_strong<l> choice{nullptr};
			assert(tc);
			/*
			assert(tc || !_ro->store().in_transaction());
			auto owner = (tc ? nullptr : std::make_unique<mtl::TransactionContext>((T*)nullptr,_ro->store().begin_transaction(),tracker.generateContext()));
			if (owner) owner->commit_on_delete = true;
			auto &ctx = (owner ? *owner : *tc);*/
			auto &ctx = *tc;
			assert(ctx.trackingContext);
			return put(tracker, ctx,t,choice);
		}
	
		void put(tracker::Tracker& tracker, mtl::TransactionContext &ctx, const T& t, std::true_type*) {
			assert(_ro);
			tracker.onWrite(ctx,_ro->store(),_ro->name(),(T*)nullptr);
			_ro->put(ctx.template get_store_context<l>(_ro->store(),"calling put() via handle").get(),t);
		}

		void put(tracker::Tracker& tracker, mtl::TransactionContext &ctx, const T& t, std::false_type*) {
			assert(_ro);
			tracker.onWrite(_ro->store(),_ro->name(),_ro->timestamp(),(T*)nullptr);
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

		static constexpr Level lnew = l;
	
		Handle<lnew,HandleAccess::read,T,SupportedOperations...> readOnly() const {
			static_assert(lnew == l || l == Level::strong,
						  "Error: request for strong read handle from causal base!");
			return Handle<lnew,HandleAccess::read,T>{_ro};
		}

		Handle<lnew,HandleAccess::write,T,SupportedOperations...> writeOnly() const {
			static_assert(lnew == l || l == Level::causal,
						  "Error: request for causal write handle from strong base!");
			Handle<lnew,HandleAccess::write,T> r{_ro};
			return r;
		}

		template<HandleAccess ha2>
		auto restrictTo() const{
			std::integral_constant<HandleAccess, ha2>* choice{nullptr};
			return restrictTo(choice);
		}

		auto restrictTo(std::integral_constant<HandleAccess,HandleAccess::read>*) const{
			return readOnly();
		}

		auto restrictTo(std::integral_constant<HandleAccess,HandleAccess::write>*) const{
			return writeOnly();
		}

		template<Level l2, HandleAccess ha2, typename T2, typename... SupportedOperations2>
		friend struct Handle;

		template<typename> friend struct mtl::Transaction;

	private:
		static void do_onwrite(mtl::TransactionContext &tc, tracker::Tracker &tr, RemoteObject<Level::strong,T> &ro){
			tr.onWrite(tc,ro.store(),ro.name(),(T*)nullptr);
		}
		static void do_onwrite(mtl::TransactionContext &tc, tracker::Tracker &tr, RemoteObject<Level::causal,T> &ro){
			tr.onWrite(ro.store(),ro.name(),ro.timestamp(),(T*)nullptr);
		}
	public:		
	
		template<HandleAccess ha2, typename T2>
		friend Handle<Level::strong,ha2,T2,SupportedOperations...> mtl::run_ast_causal(mtl::TransactionContext *, const mtl::CausalCache& cache, const mtl::CausalStore &, const Handle<Level::strong,ha2,T2,SupportedOperations...>& h);

		template<HandleAccess ha, typename T2>
		friend Handle<Level::strong,ha,T2,SupportedOperations...>
		mtl::run_ast_causal(mtl::TransactionContext *, mtl::CausalCache& cache, const mtl::CausalStore &s,
							const Handle<Level::strong,ha,T2,SupportedOperations...>& h);
	};



	template<Level l, HandleAccess ha, typename T, typename... Ops>
	constexpr Level chld_min_level_f(Handle<l,ha,T,Ops...> const * const){
		return l;
	}

	namespace mtl{
		template<unsigned long long, typename>
		struct contains_temporary;
	}

	template<unsigned long long id, Level l, HandleAccess ha, typename T, typename... Ops>
	struct mtl::contains_temporary<id,  Handle<l,ha,T,Ops...> > : std::false_type {};


	template<typename T>
	struct is_handle;

	template<Level l, HandleAccess ha, typename T, typename... Ops>
	struct is_handle<Handle<l,ha,T,Ops...> > : std::true_type {};

	template<typename T>
	struct is_handle : std::false_type {};

	namespace mtl{
		template<typename>
		struct is_ConExpr;
	}

	template<typename T, Level l, HandleAccess ha, typename... Ops>
	struct mtl::is_ConExpr<Handle<l,ha,T,Ops...> > : std::true_type {};

	template<typename T>
	struct is_not_handle : std::integral_constant<bool, !is_handle<T>::value >::type {};

	template<Level l, HandleAccess ha, typename T, typename... Ops>
	struct mtl::extract_type<Handle<l,ha,T,Ops...> > {typedef T type;};
	template<Level l, HandleAccess ha, typename T, typename... Ops>
	struct mtl::extract_type<const Handle<l,ha,T,Ops...> > {typedef T type;};
	template<Level l, HandleAccess ha, typename T,typename... Ops>
	struct mtl::extract_type<const Handle<l,ha,T,Ops...>& > {typedef T type;};
	template<Level l, HandleAccess ha, typename T,typename... Ops>
	struct mtl::extract_type<Handle<l,ha,T,Ops...>& > {typedef T type;};

	template<typename T>
	using extract_type_t = typename mtl::extract_type<T>::type ;

	template<typename H>
	struct extract_access;

	template<Level l, HandleAccess ha, typename T,typename... Ops>
	struct extract_access<Handle<l,ha,T,Ops...> > :
		std::integral_constant<HandleAccess, ha>::type {};

	template<typename T>
	struct extract_access : std::integral_constant<HandleAccess, HandleAccess::all>::type {};


	template<typename T>
	struct is_readable_handle :
		std::integral_constant<bool,
							   is_handle<T>::value &&
							   canRead<extract_access<T>::value>::value >::type {};

	template<typename T>
	struct is_writeable_handle :
		std::integral_constant<bool,
							   is_handle<T>::value &&
							   canWrite<extract_access<T>::value>::value >::type {};

	template<typename T>
	struct is_strong_handle :
		std::integral_constant<bool,
							   is_handle<T>::value &&
							   (mtl::get_level<T>::value == Level::strong) >::type {};

	template<typename T>
	struct is_causal_handle :
		std::integral_constant<bool,
							   is_handle<T>::value &&
							   (mtl::get_level<T>::value == Level::causal) >::type {};	

}
namespace mutils{
	
	template<myria::Level l, myria::HandleAccess ha, typename T,typename... Ops>
	int to_bytes(const myria::Handle<l,ha,T,Ops...>& h, char* v){
		return h.to_bytes_hndl(v);
	}

	
	template<myria::Level l, myria::HandleAccess ha, typename T,typename... Ops>
	int bytes_size(const myria::Handle<l,ha,T,Ops...> &h){
		return h.bytes_size_hndl();
	}
	
	template<myria::Level l, myria::HandleAccess ha, typename T,typename... Ops>
	void post_object(const std::function<void (char const *const, std::size_t)>&f,
					 const myria::Handle<l,ha,T,Ops...>& h){
		auto size = ::mutils::bytes_size(h);
		char buf[size];
		h.to_bytes_hndl(buf);
		f(buf,size);
	}
	
	template<myria::Level l, myria::HandleAccess ha, typename T, typename... Ops>
	void ensure_registered(const myria::Handle<l,ha,T,Ops...>& v, DeserializationManager& dm){
		ensure_registered(*v._ro,dm);
	}

    template<typename T, typename P>
	std::enable_if_t<myria::is_handle<T>::value,std::unique_ptr<T> > from_bytes(P* p, char const *v){
		return T::from_bytes(p,v);
	}
}

/*
//forward-declaring

template<typename T>
struct is_handle;
template<Level, HandleAccess, typename>
struct Handle;

template<typename T>
std::enable_if_t<is_handle<T>::value,std::unique_ptr<T> > from_bytes(char const *v);

template<Level l, HandleAccess ha, typename T>
int to_bytes(const Handle<l,ha,T>& h, char* v);

template<Level l, HandleAccess ha, typename T>
int bytes_size(const Handle<l,ha,T> &h);
	
*/
