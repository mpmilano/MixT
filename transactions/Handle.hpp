//new handle.  the old one is pretty broken.
#pragma once
#include "../BitSet.hpp"
#include "Basics.hpp"
#include "RemoteObject.hpp"
#include "Store.hpp"
#include "Tracker.hpp"
#include <memory>

template<typename,typename>
struct Operation;

//todo: move this

template<typename >
struct extract_type;

template<typename>
struct get_level;

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
public:
	Tracker &tracker;
private:
	Handle(std::shared_ptr<RemoteObject<l,T> > _ro):_ro(_ro),tracker(Tracker::global_tracker()){
		assert(tracker.registered(_ro->store()));
        assert(_ro->store().level == l);
	}
public:
	
	const int uid = gensym();
	
	typename std::conditional<canWrite<HA>::value,
							  RemoteObject<l,T>&,
							  const RemoteObject<l,T>& >::type
	remote_object() const {
		assert(_ro);
		return *_ro;
	}

	Handle():tracker(Tracker::global_tracker()) {}
	Handle(const Handle& h):_ro(h._ro),tracker(Tracker::global_tracker()){}
		
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

		assert(from_bytes(v));
	}

	int bytes_size_hndl() const {
		return sizeof(bool) + (_ro ? _ro->bytes_size() : 0);
	}

	static std::unique_ptr<Handle> from_bytes(char *v)  {
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

	const T& get() const {
		assert(_ro);
		choose_strong<l> choice {nullptr};
		assert(_ro->store().level == l);
		return get(choice);
	}
	
	const T& get(std::true_type*) const {
		tracker.onRead(_ro->store(),_ro->name());
		return _ro->get();
	}
	
	const T& get(std::false_type*) const {
		tracker.onRead(_ro->store(),_ro->name(),_ro->timestamp());
		return _ro->get();
	}
	
	Handle clone() const {
		return *this;
	}

	void put(const T& t){
		choose_strong<l> choice{nullptr};
		return put(t,choice);
	}
	
	void put(const T& t, std::true_type*) {
		assert(_ro);
		tracker.onWrite(_ro->store(),_ro->name());
		_ro->put(t);
	}

	void put(const T& t, std::false_type*) {
		assert(_ro);
		tracker.onWrite(_ro->store(),_ro->name(),_ro->timestamp());
		_ro->put(t);
	}

	bool isValid() const {
		if (!_ro) return false;
		return _ro->ro_isValid();
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

	friend struct Transaction;

	template<typename RO, typename... Args>
	static Handle<l,HA,T> make_handle(Args && ... ca){
		static_assert(std::is_base_of<RemoteObject<l,T>,RO >::value,
					  "Error: must template on valid RemoteObject extender");
		RemoteObject<l,T> *rop = new RO(std::forward<Args>(ca)...);
		std::shared_ptr<RemoteObject<l,T> > sp(rop);
		Handle<l,HA,T> ret(sp);
		return ret;
	}
	
	template<HandleAccess ha2, typename T2>
	friend Handle<Level::strong,ha2,T2> run_ast_causal(const CausalCache& cache, const CausalStore &, const Handle<Level::strong,ha2,T2>& h);

	template<HandleAccess ha, typename T2>
	friend Handle<Level::strong,ha,T2>
	run_ast_causal(CausalCache& cache, const CausalStore &s,
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


template<unsigned long long, typename>
struct contains_temporary;

template<unsigned long long id, Level l, HandleAccess ha, typename T>
struct contains_temporary<id,  Handle<l,ha,T> > : std::false_type {};

template<Level l, HandleAccess HA, typename T,
		 typename RO, typename... Args>
Handle<l,HA,T> make_handle(Args && ... ca)
{
	return Handle<l,HA,T>::template make_handle<RO, Args...>(std::forward<Args>(ca)...);
}


template<typename T>
struct is_handle;

template<Level l, HandleAccess ha, typename T>
struct is_handle<Handle<l,ha,T> > : std::true_type {};

template<typename T>
struct is_handle : std::false_type {};

template<typename>
struct is_ConExpr;

template<typename T, Level l, HandleAccess ha>
struct is_ConExpr<Handle<l,ha,T> > : std::true_type {};

template<typename T>
struct is_not_handle : std::integral_constant<bool, !is_handle<T>::value >::type {};

template<Level l, HandleAccess ha, typename T>
struct extract_type<Handle<l,ha,T> > {typedef T type;};

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
						   (get_level<T>::value == Level::strong) >::type {};

template<typename T>
struct is_causal_handle :
	std::integral_constant<bool,
						   is_handle<T>::value &&
						   (get_level<T>::value == Level::causal) >::type {};

template<typename T>
std::enable_if_t<is_handle<T>::value,std::unique_ptr<T> > from_bytes(char *v){
	return T::from_bytes(v);
}

template<Level l, HandleAccess ha, typename T>
int to_bytes(const Handle<l,ha,T>& h, char* v){
	return h.to_bytes_hndl(v);
}

template<Level l, HandleAccess ha, typename T>
int bytes_size(const Handle<l,ha,T> &h){
	return h.bytes_size_hndl();
}
