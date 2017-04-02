#pragma once
#include "SerializationSupport.hpp"

namespace myria{ namespace mtl{
	template<typename T>
	context_ptr<const type_check<is_type_holder,T> > from_bytes_noalloc(DeserializationManager* dsm, const char * v, context_ptr<T> = context_ptr<T>{} ){
		using namespace mutils;
		auto ret = new T;
		ret->t = *from_bytes_noalloc<DECT(ret->t)>(dsm,v);
		v += mutils::bytes_size(ret->t);
		ret->curr_pos = *from_bytes_noalloc<DECT(ret->curr_pos)>(dsm,v);
		v += mutils::bytes_size(ret->curr_pos);
		ret->rollback_size = *from_bytes_noalloc<DECT(ret->rollback_size)>(dsm,v);
		v += mutils::bytes_size(ret->rollback_size);
		ret->bound = *from_bytes_noalloc<DECT(ret->bound)>(dsm,v);
		v += mutils::bytes_size(ret->bound);
		return context_ptr<const T>{ret};
	}

	template<typename T, char... str>
	std::size_t to_bytes(const type_holder<T,str...>& t, char * _v){
		auto *v = _v;
		mutils::to_bytes(t.t,v);
		v += mutils::bytes_size(t.t);
		mutils::to_bytes(t.curr_pos,v);
		v += mutils::bytes_size(t.curr_pos);
		mutils::to_bytes(t.rollback_size,v);
		v += mutils::bytes_size(t.rollback_size);
		mutils::to_bytes(t.bound,v);
		v += mutils::bytes_size(t.bound);
		return v - _v;
	}

	template<typename T, char... str>
	std::size_t bytes_size(const type_holder<T,str...>& t){
		return 
			mutils::bytes_size(t.t)+
			mutils::bytes_size(t.curr_pos)+
			mutils::bytes_size(t.rollback_size)+
			mutils::bytes_size(t.bound);
	}
	
	template<typename T>
	context_ptr<const type_check<is_remote_holder,T> > from_bytes_noalloc(DeserializationManager* dsm, const char * v, context_ptr<T> = context_ptr<T>{}){
		auto ret = new T;
		ret->t = *mutils::from_bytes_noalloc<DECT(ret->t)>(dsm,v);
		v += mutils::bytes_size(ret->t);
		ret->curr_pos = *mutils::from_bytes_noalloc<DECT(ret->curr_pos)>(dsm,v);
		v += mutils::bytes_size(ret->curr_pos);
		ret->rollback_size = *mutils::from_bytes_noalloc<DECT(ret->rollback_size)>(dsm,v);
		v += mutils::bytes_size(ret->rollback_size);
		ret->bound = *mutils::from_bytes_noalloc<DECT(ret->bound)>(dsm,v);
		v += mutils::bytes_size(ret->bound);
		ret->initialized = *mutils::from_bytes_noalloc<DECT(ret->initialized)>(dsm,v);
		v += mutils::bytes_size(ret->initialized);
		ret->list_usable = *mutils::from_bytes_noalloc<DECT(ret->list_usable)>(dsm,v);
		v += mutils::bytes_size(ret->list_usable);
		ret->handle = *mutils::from_bytes_noalloc<DECT(ret->handle)>(dsm,v);
		v += mutils::bytes_size(ret->handle);
		return mutils::context_ptr<const T>{ret};
	}

	template<typename T>
	context_ptr<const type_check<is_value_holder,T> > from_bytes_noalloc(DeserializationManager* dsm, const char * v, context_ptr<T> = context_ptr<T>{}){
		auto ret = new T;
		ret->t = *mutils::from_bytes_noalloc<DECT(ret->t)>(dsm,v);
		v += mutils::bytes_size(ret->t);
		ret->pre_phase_t = *mutils::from_bytes_noalloc<DECT(ret->pre_phase_t)>(dsm,v);
		v += mutils::bytes_size(ret->pre_phase_t);
		return mutils::context_ptr<const T>{ret};
	}

	template<typename T, char... str>
	std::size_t to_bytes(const value_holder<T,str...>& t, char * _v){
		auto *v = _v;
		mutils::to_bytes(t.t,v);
		v += mutils::bytes_size(t.t);
		mutils::to_bytes(t.pre_phase_t,v);
		v += mutils::bytes_size(t.pre_phase_t);
		return v - _v;
	}

	template<typename T, char... str>
	std::size_t bytes_size(const value_holder<T,str...>& t){
		return mutils::bytes_size(t.t) + mutils::bytes_size(t.pre_phase_t);
	}

	template<typename T, char... str>
	std::size_t to_bytes(const remote_holder<T,str...>& t, char * _v){
		auto *v = _v;
		mutils::to_bytes(t.t,v);
		v += mutils::bytes_size(t.t);
		mutils::to_bytes(t.curr_pos,v);
		v += mutils::bytes_size(t.curr_pos);
		mutils::to_bytes(t.rollback_size,v);
		v += mutils::bytes_size(t.rollback_size);
		mutils::to_bytes(t.bound,v);
		v += mutils::bytes_size(t.bound);
		mutils::to_bytes(t.initialized,v);
		v += mutils::bytes_size(t.initialized);
		mutils::to_bytes(t.list_usable,v);
		v += mutils::bytes_size(t.list_usable);
		mutils::to_bytes(t.handle,v);
		v += mutils::bytes_size(t.handle);
		return v - _v;
	}

	template<typename T, char... str>
	std::size_t bytes_size(const remote_holder<T,str...>& t){
		return mutils::bytes_size(t.t)+
			mutils::bytes_size(t.curr_pos)+
			mutils::bytes_size(t.rollback_size)+
			mutils::bytes_size(t.bound)+
			mutils::bytes_size(t.initialized)+
			mutils::bytes_size(t.list_usable)+
			mutils::bytes_size(t.handle);
	}

#ifndef NDEBUG
	template<typename T, char... str>
	void ensure_registered(const remote_holder<T,str...>& , mutils::DeserializationManager&){}
	template<typename T, char... str>
	void ensure_registered(const value_holder<T,str...>& , mutils::DeserializationManager&){}
		template<typename T, char... str>
	void ensure_registered(const type_holder<T,str...>& , mutils::DeserializationManager&){}
#endif

		template<typename T>
		std::size_t bytes_size_reflect(const T& t){
			return bytes_size(t);
		}
	}}
