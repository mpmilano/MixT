#pragma once
#include "SerializationSupport.hpp"

namespace mutils{
	template<typename T>
	context_ptr<type_check<is_type_holder>,T> from_bytes_noalloc(DeserializationManager* dsm, char * v){
		auto ret = new T;
		ret->t = *from_bytes_noalloc<DECT(ret->t)>(dsm,v);
		v += bytes_size(ret->t);
		ret->curr_pos = *from_bytes_noalloc<DECT(ret->curr_pos)>(dsm,v);
		v += bytes_size(ret->curr_pos);
		ret->rollback_size = *from_bytes_noalloc<DECT(ret->rollback_size)>(dsm,v);
		v += bytes_size(ret->rollback_size);
		ret->bound = *from_bytes_noalloc<DECT(ret->bound)>(dsm,v);
		v += bytes_size(ret->bound);
		return context_ptr<T>{ret};
	}

	template<typename T, char... str>
	std::size_t to_bytes(const type_holder<T,str...>& t, char * _v){
		auto *v = _v;
		to_bytes(t.t,v);
		v += bytes_size(t.t);
		to_bytes(t.curr_pos,v);
		v += bytes_size(t.curr_pos);
		to_bytes(t.rollback_size,v);
		v += bytes_size(t.rollback_size);
		to_bytes(t.bound,v);
		v += bytes_size(t.bound);
		return v - _v;
	}

	template<typename T, char... str>
	std::size_t bytes_size(const type_holder<T,str...>& t){
		return 
			bytes_size(t.t)+
			bytes_size(t.curr_pos)+
			bytes_size(t.rollback_size)+
			bytes_size(t.bound);
	}
	
	template<typename T>
	context_ptr<type_check<is_remote_holder>,T> from_bytes_noalloc(DeserializationManager* dsm, char * v){
		auto ret = new T;
		ret->t = *from_bytes_noalloc<DECT(ret->t)>(dsm,v);
		v += bytes_size(ret->t);
		ret->curr_pos = *from_bytes_noalloc<DECT(ret->curr_pos)>(dsm,v);
		v += bytes_size(ret->curr_pos);
		ret->rollback_size = *from_bytes_noalloc<DECT(ret->rollback_size)>(dsm,v);
		v += bytes_size(ret->rollback_size);
		ret->bound = *from_bytes_noalloc<DECT(ret->bound)>(dsm,v);
		v += bytes_size(ret->bound);
		ret->initialized = *from_bytes_noalloc<DECT(ret->initialized)>(dsm,v);
		v += bytes_size(ret->initialized);
		ret->list_usable = *from_bytes_noalloc<DECT(ret->list_usable)>(dsm,v);
		v += bytes_size(ret->list_usable);
		ret->handle = *from_bytes_noalloc<DECT(ret->handle)>(dsm,v);
		v += bytes_size(ret->handle);
		return context_ptr<T>{ret};
	}

	template<typename T, char... str>
	std::size_t to_bytes(const remote_holder<T,str...>& t, char * _v){
		auto *v = _v;
		to_bytes(t.t,v);
		v += bytes_size(t.t);
		to_bytes(t.curr_pos,v);
		v += bytes_size(t.curr_pos);
		to_bytes(t.rollback_size,v);
		v += bytes_size(t.rollback_size);
		to_bytes(t.bound,v);
		v += bytes_size(t.bound);
		to_bytes(t.initialized,v);
		v += bytes_size(t.initialized);
		to_bytes(t.list_usable,v);
		v += bytes_size(t.list_usable);
		to_bytes(t.handle,v);
		v += bytes_size(t.handle);
		return v - _v;
	}

	template<typename T, char... str>
	std::size_t bytes_size(const remote_holder<T,str...>& t){
		return bytes_size(t.t)+
			bytes_size(t.curr_pos)+
			bytes_size(t.rollback_size)+
			bytes_size(t.bound)+
			bytes_size(t.initialized)+
			bytes_size(t.list_usable)+
			bytes_size(t.handle);
	}
}
