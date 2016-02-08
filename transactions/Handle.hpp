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

		template<HandleAccess ha2, typename T2>
		Handle<Level::strong,ha2,T2> run_ast_causal(TransactionContext *ctx, const mtl::CausalCache& cache, const mtl::CausalStore &, const Handle<Level::strong,ha2,T2>& h);
		
		template<HandleAccess ha, typename T2>
		Handle<Level::strong,ha,T2>
		run_ast_causal(TransactionContext *ctx, mtl::CausalCache& cache, const mtl::CausalStore &s,
					   const Handle<Level::strong,ha,T2>& h);


	}

	template<Level l, HandleAccess HA>
	struct GenericHandle {};

	template<unsigned long long id, Level l, HandleAccess ha>
	std::nullptr_t find_usage(const GenericHandle<l,ha>&){
		return nullptr;
	}
	
	template<Level l, HandleAccess HA, typename T>
	struct Handle : public GenericHandle<l,HA> {

	private:
		const std::shared_ptr<RemoteObject<l,T> > _ro;
	private:
		Handle(std::shared_ptr<RemoteObject<l,T> > _ro):_ro(_ro){
			assert(_ro->store().level == l);
		}
	public:
	
		const int uid = mutils::gensym();
	
		typename std::conditional<canWrite<HA>::value,
								  RemoteObject<l,T>&,
								  const RemoteObject<l,T>& >::type
		remote_object() const {
			assert(_ro);
			return *_ro;
		}

		Handle() {}
		Handle(const Handle& h):_ro(h._ro){}
		
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

			auto b = from_bytes(v);
			assert(b);
		}

		int bytes_size_hndl() const {
			return sizeof(bool) + (_ro ? _ro->bytes_size() : 0);
		}

		static std::unique_ptr<Handle> from_bytes(char const *v)  {
			//for de-serializing.
			assert(v);
			RemoteObject<l,T> *stupid = nullptr;
			if (((bool*)v)[0]) {
				auto ro = from_bytes_stupid(stupid,v + sizeof(bool) );
				assert(ro);
				return std::unique_ptr<Handle>(
					new Handle(
						std::shared_ptr<RemoteObject<l,T> >(ro.release())));
			}
			else return std::make_unique<Handle>();
		}

		const T& get(tracker::Tracker& tracker, mtl::TransactionContext *tc) const {
			assert(_ro);
			choose_strong<l> choice {nullptr};
			assert(_ro->store().level == l);
			assert(tc || !_ro->store().in_transaction());
			auto owner = (tc ? nullptr : std::make_unique<mtl::TransactionContext>((void*)nullptr,_ro->store().begin_transaction(),tracker.generateContext()));
			if (owner) owner->commit_on_delete = true;
			auto &ctx = (owner ? *owner : *tc);
			assert(ctx.trackingContext);

			//If the Transacion Context does not yet exist for this store, we create it now.
			auto &store_ctx = *ctx.template get_store_context<l>(_ro->store());

			return get(choice,tracker,store_ctx, *ctx.trackingContext, _ro->get(&store_ctx));
		}
	
		const T& get(std::true_type*, tracker::Tracker& tracker, mtl::StoreContext<l>& ctx, tracker::TrackingContext &trkc, const T& ret) const {
			tracker.afterRead(ctx,trkc,_ro->store(),_ro->name());
			return ret;
		}
	
		const T& get(std::false_type*, tracker::Tracker& tracker, mtl::StoreContext<l>& ctx, tracker::TrackingContext &trkc, const T& ret) const {
			mutils::AtScopeEnd ase{[&](){tracker.afterRead(trkc,_ro->store(),_ro->name(),_ro->timestamp(),_ro->bytes());}};
			if (tracker.waitForRead(trkc,_ro->store(),_ro->name(),_ro->timestamp())){
				return ret;
			}
			else return _ro->get(&ctx);
		}
	
		Handle clone() const {
			return *this;
		}

		void put(tracker::Tracker& tracker, mtl::TransactionContext *tc, const T& t){
			choose_strong<l> choice{nullptr};
			assert(tc || !_ro->store().in_transaction());
			auto owner = (tc ? nullptr : std::make_unique<mtl::TransactionContext>(nullptr,_ro->store().begin_transaction(),tracker.generateContext()));
			if (owner) owner->commit_on_delete = true;
			auto &ctx = (owner ? *owner : *tc);
			assert(ctx.trackingContext);
			return put(tracker, *ctx.template get_store_context<l>(_ro->store()),*ctx.trackingContext,t,choice);
		}
	
		void put(tracker::Tracker& tracker, mtl::StoreContext<l> &ctx, tracker::TrackingContext& trk, const T& t, std::true_type*) {
			assert(_ro);
			tracker.onWrite(ctx,trk,_ro->store(),_ro->name());
			_ro->put(ctx,t);
		}

		void put(tracker::Tracker& tracker, mtl::StoreContext<l> &ctx, tracker::TrackingContext& trk, const T& t, std::false_type*) {
			assert(_ro);
			tracker.onWrite(_ro->store(),_ro->name(),_ro->timestamp());
			_ro->put(ctx,t);
		}

		bool isValid(mtl::TransactionContext *ctx) const {
			if (!_ro) return false;
			auto *ptr = (ctx ? ctx->template get_store_context<l>(_ro->store()).get() : nullptr);
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
	
		Handle<lnew,HandleAccess::read,T> readOnly() const {
			static_assert(lnew == l || l == Level::strong,
						  "Error: request for strong read handle from causal base!");
			return Handle<lnew,HandleAccess::read,T>{_ro};
		}

		Handle<lnew,HandleAccess::write,T> writeOnly() const {
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

		template<Level l2, HandleAccess ha2, typename T2>
		friend struct Handle;

		template<typename> friend struct mtl::Transaction;

	private:
		static void do_onwrite(mtl::TransactionContext &tc, tracker::Tracker &tr, RemoteObject<Level::strong,T> &ro){
			tr.onWrite(tc,ro.store(),ro.name());
		}
		static void do_onwrite(mtl::TransactionContext &tc, tracker::Tracker &tr, RemoteObject<Level::causal,T> &ro){
			tr.onWrite(ro.store(),ro.name(),ro.timestamp());
		}
	public:

		template<typename RO, typename... Args>
		static Handle<l,HA,T> make_handle(tracker::Tracker &trk, mtl::TransactionContext *tc, Args && ... ca){
			static_assert(std::is_base_of<RemoteObject<l,T>,RO >::value,
						  "Error: must template on valid RemoteObject extender");
			RemoteObject<l,T> *rop = new RO(std::forward<Args>(ca)...);
			std::shared_ptr<RemoteObject<l,T> > sp(rop);
			Handle<l,HA,T> ret(sp);
			assert(tc || !rop->store().in_transaction());
			//make a transaction with no environment expressions for only the requested store
			auto owner = (tc ? nullptr : std::make_unique<mtl::TransactionContext>((void*)nullptr,rop->store().begin_transaction(),trk.generateContext()));
			if (owner) owner->commit_on_delete = true;
			auto &ctx = (tc ? *tc : *owner);
			do_onwrite(ctx,trk,*rop);
			return ret;
		}
	
		template<HandleAccess ha2, typename T2>
		friend Handle<Level::strong,ha2,T2> mtl::run_ast_causal(mtl::TransactionContext *, const mtl::CausalCache& cache, const mtl::CausalStore &, const Handle<Level::strong,ha2,T2>& h);

		template<HandleAccess ha, typename T2>
		friend Handle<Level::strong,ha,T2>
		mtl::run_ast_causal(mtl::TransactionContext *, mtl::CausalCache& cache, const mtl::CausalStore &s,
							const Handle<Level::strong,ha,T2>& h);

		template<typename, typename>
		friend struct Operation;

	
		/*
		//TODO: same treatment as in Operate
		template<typename... Args>
		auto op(Operation<bool(*) (RemoteObject<l,T>*, cr_add<Args>...)> (*fp) (RemoteObject<l,T>*, cr_add<Args>...), Args && ... a){
		return fp(&remote_object(),std::forward<Args>(a)...)(*this, std::forward<Args>(a)...);
		}
		*/
	};



	template<Level l, HandleAccess ha, typename T>
	constexpr Level chld_min_level_f(Handle<l,ha,T> const * const){
		return l;
	}

	namespace mtl{
		template<unsigned long long, typename>
		struct contains_temporary;
	}

	template<unsigned long long id, Level l, HandleAccess ha, typename T>
	struct mtl::contains_temporary<id,  Handle<l,ha,T> > : std::false_type {};

	template<Level l, HandleAccess HA, typename T,
			 typename RO, typename... Args>
	Handle<l,HA,T> make_handle(tracker::Tracker &trk, mtl::TransactionContext *tc, Args && ... ca)
	{
		return Handle<l,HA,T>::template make_handle<RO, Args...>(trk,tc,std::forward<Args>(ca)...);
	}


	template<typename T>
	struct is_handle;

	template<Level l, HandleAccess ha, typename T>
	struct is_handle<Handle<l,ha,T> > : std::true_type {};

	template<typename T>
	struct is_handle : std::false_type {};

	namespace mtl{
		template<typename>
		struct is_ConExpr;
	}

	template<typename T, Level l, HandleAccess ha>
	struct mtl::is_ConExpr<Handle<l,ha,T> > : std::true_type {};

	template<typename T>
	struct is_not_handle : std::integral_constant<bool, !is_handle<T>::value >::type {};

	template<Level l, HandleAccess ha, typename T>
	struct mtl::extract_type<Handle<l,ha,T> > {typedef T type;};

	template<typename H>
	struct extract_access;

	template<Level l, HandleAccess ha, typename T>
	struct extract_access<Handle<l,ha,T> > :
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
	
	template<myria::Level l, myria::HandleAccess ha, typename T>
	int to_bytes(const myria::Handle<l,ha,T>& h, char* v){
		return h.to_bytes_hndl(v);
	}
	
	template<myria::Level l, myria::HandleAccess ha, typename T>
	int bytes_size(const myria::Handle<l,ha,T> &h){
		return h.bytes_size_hndl();
	}

	template<typename T>
	std::enable_if_t<myria::is_handle<T>::value,std::unique_ptr<T> > from_bytes(char const *v){
		return T::from_bytes(v);
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
