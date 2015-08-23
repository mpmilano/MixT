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

struct HandleAbbrev{
	
	static constexpr std::true_type* CompatibleWithBitset = nullptr;
	const BitSet<HandleAbbrev>::member_t value;
	typedef decltype(value) itype;
	
	//dear programmer; it's on you to make sure that this is true.
	static constexpr int numbits = sizeof(decltype(value));
	
	operator decltype(value)() const {
		return value;
	}
	HandleAbbrev(decltype(value) v):value(v){}
	
	
	bool operator<(const HandleAbbrev& o) const {
		return value < o.value;
	}
	//idea; we use this for tracking the ReadSet.
};

template<Level l, HandleAccess HA>
struct GenericHandle {};

template<unsigned long long id, Level l, HandleAccess ha>
std::nullptr_t find_usage(const GenericHandle<l,ha>&){
	return nullptr;
}

template<Level l, HandleAccess HA, typename T>
struct Handle : public GenericHandle<l,HA>, public ByteRepresentable {

private:
	const std::shared_ptr<RemoteObject<T> > _ro;
	Tracker &t;
	Handle(std::shared_ptr<RemoteObject<T> > _ro):_ro(_ro),t(Tracker::global_tracker()){
		t.registerStore(_ro->store());
	}
public:
	
	const int uid = gensym();
	
	typename std::conditional<canWrite<HA>::value,
							  RemoteObject<T>&,
							  const RemoteObject<T>& >::type
	remote_object() const {
		assert(_ro);
		return *_ro;
	}

	Handle():t(Tracker::global_tracker()) {}
		
	static constexpr Level level = l;
	static constexpr HandleAccess ha = HA;
	typedef T stored_type;

	void* to_bytes() const {
		//for serialization
		return _ro->to_bytes();
	}

	int bytes_size() const {
		return _ro->bytes_size();
	}

	template<typename RObject>
	static Handle from_bytes(void *v, int size)  {
		//for de-serializing.
		return Handle{std::make_shared<RObject>(RObject::from_bytes(v,size))};
	}
	
	const T& get() const {
		//TODO: causal tracking
		assert(_ro);
		return _ro->get();
	}
	
	Handle clone() const {
		return *this;
	}
	
	void put(const T& t) {
		//TODO: causal tracking
		assert(_ro);
		_ro->put(t);
	}

	bool isValid() const {
		return _ro->isValid();
	}
	
	operator HandleAbbrev() const {
		HandleAbbrev::itype i = 1;
		return i << _ro->id;
	}
	
	HandleAbbrev abbrev() const {
		return *this;
	}

	template<Level lnew = l>
	Handle<lnew,HandleAccess::read,T> readOnly() const {
		static_assert(lnew == l || l == Level::strong,
					  "Error: request for strong read handle from causal base!");
		return Handle<lnew,HandleAccess::read,T>{_ro};
	}

	template<Level lnew = l>
	Handle<lnew,HandleAccess::write,T> writeOnly() const {
		static_assert(lnew == l || l == Level::causal,
					  "Error: request for causal write handle from strong base!");
		Handle<lnew,HandleAccess::write,T> r{_ro};
		return r;
	}

	template<Level l2, HandleAccess ha2, typename T2>
	friend struct Handle;

	template<typename RO, typename... Args>
	static Handle<l,HA,T> make_handle(Args && ... ca){
		static_assert(std::is_base_of<RemoteObject<T>,RO >::value,
					  "Error: must template on valid RemoteObject extender");
		RemoteObject<T> *rop = new RO(std::forward<Args>(ca)...);
		std::shared_ptr<RemoteObject<T> > sp(rop);
		Handle<l,HA,T> ret(sp);
		return ret;
	}
	
	template<HandleAccess ha2, typename T2>
	friend Handle<Level::strong,ha2,T2> run_ast_causal(const Store &cache, const Store &, const Handle<Level::strong,ha2,T2>& h);

	template<Level l2, HandleAccess ha2, typename T2>
	friend void markInTransaction(Store &s, const Handle<l2,ha2,T2> &h);

	template<HandleAccess ha, typename T2>
	friend Handle<Level::strong,ha,T2>
	run_ast_causal(Store &cache, const Store &s,
				   const Handle<Level::strong,ha,T2>& h);

	
/*
	//TODO: same treatment as in Operate
	template<typename... Args>
	auto op(Operation<bool(*) (RemoteObject<T>*, cr_add<Args>...)> (*fp) (RemoteObject<T>*, cr_add<Args>...), Args && ... a){
		return fp(&remote_object(),std::forward<Args>(a)...)(*this, std::forward<Args>(a)...);
	}
*/
};

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

template<Level l, HandleAccess ha, typename T>
auto get_ReadSet(const Handle<l,ha,T> &h){
	return BitSet<HandleAbbrev>(h.abbrev());
}

template<Level l, HandleAccess ha, typename T>
std::ostream & operator<<(std::ostream &os, const Handle<l,ha,T>&){
	return os << "Handle<" << levelStr<l>() << ">";
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

template<typename>
struct extract_type;

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
