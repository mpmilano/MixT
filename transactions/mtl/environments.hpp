#pragma once

#include "CTString.hpp"
#include <type_traits>
#include <vector>
#include <cassert>
#include <iostream>
#include "mtlutils.hpp"
#include "top.hpp"
#include "Basics.hpp"
#include "Handle.hpp"
#include "TransactionContext.hpp"

namespace myria {

namespace mtl {

template <char... str>
using String = mutils::String<str...>;
}

template <typename>
struct Label;

namespace mtl {

enum class type_location
{
  local,
		remote,
		remote_isValid
};
template <typename, typename, typename, type_location>
struct type_binding;

template <typename T, char... str>
struct value_holder
{

	using held_type = T;
  T t;
  value_holder(T t)
    : t(t)
  {
  }
  value_holder() = default;
	
  using type = T;
  using name = String<str...>;
  template <typename TransactionContext>
  T& get(TransactionContext&)
  {
    return t;
  }
  template <typename TransactionContext>
  value_holder& push(TransactionContext&, const T& t2)
  {
    t = t2;
    return *this;
  }
	template <typename TransactionContext>
  value_holder& bind(TransactionContext&, const T& t2)
  {
    t = t2;
    return *this;
  }
	bool reset_index() { return true; }
	bool begin_phase(){
		return reset_index();
	}

  template <typename Other>
  static constexpr mutils::mismatch get_holder(value_holder*, std::enable_if_t<!std::is_same<Other, name>::value, Other>)
  {
    return mutils::mismatch{};
  };
  template <typename>
  static constexpr value_holder* get_holder(value_holder* _this, name)
  {
    return _this;
  }
  using value = value_holder;
};
	template<typename> struct is_value_holder;
	template <typename T, char... str>
	struct is_value_holder<value_holder<T,str...> > : public std::true_type{};
	template <typename T>
	struct is_value_holder : public std::false_type{};

template <typename T, char... str>
using value = value_holder<T, str...>;

template <typename, typename>
struct value_with_stringname_str;
template <typename T, char... str>
struct value_with_stringname_str<T, String<str...>>
{
  using type = value<T, str...>;
};

template <typename T, typename N>
using value_with_stringname = typename value_with_stringname_str<T, N>::type;

template <typename T, char... str>
struct type_holder
{

	using held_type = T;
	using name = String<str...>;
	
  std::vector<T> t;
  int curr_pos{ -1 };
  bool bound = false;

  type_holder() = default;
	
  type_holder(value<T, str...> v) { bind(v.t); }

  bool reset_index()
  {
    curr_pos = -1;
    return true;
  }
	bool begin_phase(){
		return reset_index();
	}

  template <typename TranCtx, typename... Args>
  type_holder& push(TranCtx&, Args&&... args)
  {
    t.emplace_back(std::forward<Args>(args)...);
    ++curr_pos;
    return *this;
  }

	template <typename TransactionContext>
  type_holder& bind(TransactionContext&, T _t)
  {
    bound = true;
    t.emplace_back(_t);
    ++curr_pos;
		assert(curr_pos < (int)t.size());
    return *this;
  }

  type_holder& increment()
  {
    ++curr_pos;
    return *this;
  }
  template <typename TranCtx>
  T get(TranCtx&)
  {
		assert(curr_pos < (int)t.size());
    return t[curr_pos];
  }

  template <typename Other>
  static constexpr mutils::mismatch get_holder(type_holder*, std::enable_if_t<!std::is_same<Other, name>::value, Other>)
  {
    return mutils::mismatch{};
  };
  template <typename>
  static constexpr type_holder* get_holder(type_holder* _this, name)
  {
    return _this;
  }

  using value = value_holder<T, str...>;
};

		template<typename> struct is_type_holder;
	template <typename T, char... str>
	struct is_type_holder<type_holder<T,str...> > : public std::true_type{};
	template <typename T>
	struct is_type_holder : public std::false_type{};

	template<typename, typename stored>
	struct remote_map_holder {
		std::map<Name,type_holder<stored> > super;
		bool reset_index(){
			for (auto& holder : super){
				holder.second.reset_index();
			}
			return true;
		}

		bool begin_phase(){
			for (auto& holder : super){
				holder.second.begin_phase();
			}
			return true;
		}
	};
	
	template <typename T, typename stored, char... str>
	struct remote_holder : public virtual remote_map_holder<T,stored>
{

  static_assert(is_handle<T>::value);

	//The idea is to protect against aliasing; we want
	//every binding site of the same handle to use the same type_holder.
	using super_t = type_holder<stored>;
	using remote_map_holder<T,stored>::super;
	std::vector<T> handle;
	int curr_pos{-1};

	using name = typename super_t::name;

	remote_holder() = default;
	remote_holder& operator=(const remote_holder& rh){
		handle = rh.handle;
		curr_pos = rh.curr_pos;
		if (rh.super.size() > 0)
			remote_map_holder<T,stored>::operator=(rh);
		return *this;
	}
	remote_holder(const remote_holder& rh)
		:handle(rh.handle),curr_pos(rh.curr_pos){
		super = rh.super;
	}

  template <typename Other>
  static constexpr mutils::mismatch get_holder(remote_holder*, std::enable_if_t<!std::is_same<Other, name>::value, Other>)
  {
    return mutils::mismatch{};
  };
  template <typename>
  static constexpr remote_holder* get_holder(remote_holder* _this, name)
  {
    return _this;
  }

	//reset index + begin_phase are universal
	bool reset_index(){
		curr_pos = -1;
		return remote_map_holder<T,stored>::reset_index();
	}

	bool begin_phase(){
		return reset_index() && remote_map_holder<T,stored>::begin_phase();
	}

	void increment()
  {
		if (curr_pos >= 0) {
			assert((int)handle.size() > curr_pos);
			this_super().increment();
		}
  }
	
	void increment_remote()
  {
		++curr_pos;
		assert(curr_pos < ((int)handle.size()));
  }

protected:
	void read_tracking_actions(_PhaseContext<typename T::label, true>& tc){
#ifdef TRACK
		tc.trk_ctx.trk.checkForTombstones(tc,get_remote(tc).name());
		tc.trk_ctx.trk.record_timestamp(tc,get_remote(tc).timestamp());
#else
		(void) tc;
#endif
	}

	void read_tracking_actions(_PhaseContext<typename T::label, false>&){}

	void bind_common(T& t){
		handle.emplace_back(t);
		++curr_pos;
		assert(curr_pos < ((int)handle.size()));
	}

	auto& this_super(){
		return super[handle[curr_pos].name()];
	}
	
public:

  void bind(PhaseContext<typename T::label>& tc, T t)
  {
		bind_common(t);
		this_super().bind(tc,*handle[curr_pos].get(&tc));
		read_tracking_actions(tc);
    return *this;
  }

  template <typename... Args>
  remote_holder& push(PhaseContext<typename T::label>& tc, Args&&... args)
  {
    this_super().push(tc, std::forward<Args>(args)...);
    handle.back().put(&tc, super.t.back());
    return *this;
  }

  template <typename TransactionContext>
  auto get_remote(TransactionContext&){
    return handle.at(curr_pos);
  }

  template <typename TransactionContext>
  auto get(TransactionContext& tc)
  {
		assert(super.count(handle.at(curr_pos)));
		return this_super().get(tc);
  }

  using value = remote_holder;
};
	
template <typename T, char... str>
struct remote_isValid_holder : public remote_holder<T,bool,str...>
{

  static_assert(is_handle<T>::value);

	//The idea is to protect against aliasing; we want
	//every binding site of the same handle to use the same type_holder.
	using super_t = remote_holder<T,bool,str...>;

	using name = typename super_t::name;

	remote_isValid_holder() = default;
	remote_isValid_holder& operator=(const remote_isValid_holder& rh){
		super_t::operator=(rh);
		return *this;
	}
	
	remote_isValid_holder(const remote_isValid_holder& rh)
		:super_t(rh){}

  template <typename Other>
  static constexpr mutils::mismatch get_holder(remote_isValid_holder*, std::enable_if_t<!std::is_same<Other, name>::value, Other>)
  {
    return mutils::mismatch{};
  };
  template <typename>
  static constexpr remote_isValid_holder* get_holder(remote_isValid_holder* _this, name)
  {
    return _this;
  }

	//reset index + begin_phase are universal
	using super_t::reset_index;
	using super_t::begin_phase;
	using super_t::increment;
	using super_t::increment_remote;
	using super_t::this_super;

  void bind(PhaseContext<typename T::label>& tc, T t)
  {
		this->bind_common(t);
		bool is_valid = this->handle[this->curr_pos].isValid(&tc);
		this_super().bind(tc,is_valid);
		if (is_valid) this->read_tracking_actions(tc);
    return *this;
  }

  template <typename... Args>
  void push(PhaseContext<typename T::label>&, Args&&...)
  {
		static_assert((std::is_same<Args,void>::value && ... && true),"Error isValid is not assignable");
  }

  template <typename TransactionContext>
  auto get_remote(TransactionContext& tc){
		return super_t::template get_remote<TransactionContext>(tc);
  }

  template <typename TransactionContext>
  auto get(TransactionContext& tc)
  {
		return super_t::template get<TransactionContext>(tc);
  }

  using value = remote_isValid_holder;
};

	template<typename> struct is_remote_holder;
	template <typename T, typename v, char... str>
	struct is_remote_holder<remote_holder<T,v,str...> > : public std::true_type{};
	template <typename T>
	struct is_remote_holder : public std::false_type{};
	
}
}
namespace mutils {

template <typename t, char... str>
struct string_of<myria::mtl::type_holder<t, str...>>
{
  std::string value;
  string_of()
    : value([] {
      std::stringstream o;
      print_varname(o, String<str...>{});
      return o.str();
    }())
  {
  }
};

template <typename t, char... str>
struct string_of<myria::mtl::value_holder<t, str...>>
{
  std::string value;
  string_of()
    : value([] {
      std::stringstream o;
      print_varname(o, String<str...>{});
      return o.str();
    }())
  {
  }
};

	template <typename t, typename v, char... str>
	struct string_of<myria::mtl::remote_holder<t, v, str...>>
{
  std::string value;
  string_of()
    : value([] {
      std::stringstream o;
      print_varname(o, String<str...>{});
      return o.str();
    }())
  {
  }
};

template <char s1, char s2>
struct string_of<String<'a', 'n', 'o', 'r', 'm', s1, s2>>
{
  std::string value = std::string{ "anorm(" } + std::to_string((int)s1) + std::string{ "," } + std::to_string((int)s2) + std::string{ ")" };
};
}
namespace myria {
namespace mtl {

template <typename, typename, typename>
struct type_binding_super;

template <typename T, typename l, char... str>
struct type_binding_super<String<str...>, T, Label<l>>
{

  using name = String<str...>;
  using label = Label<l>;

  constexpr type_binding_super() = default;
};

template <typename T, typename l, char... str>
struct type_binding<String<str...>, T, Label<l>, type_location::local> : public type_binding_super<String<str...>, T, Label<l>>
{
  using holder = type_holder<T, str...>;

  constexpr type_binding() = default;
  using super = type_binding_super<String<str...>, T, Label<l>>;
  using name = typename super::name;
  using label = typename super::label;
  using type = T;

  template <typename Other>
  static constexpr mutils::mismatch get_binding(std::enable_if_t<!std::is_same<Other, name>::value, Other>)
  {
    return mutils::mismatch{};
  }
  template <typename>
  static constexpr type_binding get_binding(name)
  {
    return type_binding{};
  }
  template <typename T2>
  static constexpr auto get_binding2(const T2& t)
  {
    return get_binding<T2>(t);
  }
};

template <typename T, typename l, char... str>
struct type_binding<String<str...>, T, Label<l>, type_location::remote> : public type_binding_super<String<str...>, typename T::type, Label<l>>
{
  static_assert(is_handle<T>::value);
	static_assert(!std::is_same<T,typename T::type>::value);

  constexpr type_binding() = default;
	using super = type_binding_super<String<str...>, typename T::type, Label<l> >;
  using name = typename super::name;
  using label = typename super::label;

  using holder = remote_holder<T, typename T::type, str...>;
  using type = typename T::type;

  template <typename Other>
  static constexpr mutils::mismatch get_binding(std::enable_if_t<!std::is_same<Other, name>::value, Other>)
  {
    return mutils::mismatch{};
  }
  template <typename>
  static constexpr type_binding get_binding(name)
  {
    return type_binding{};
  }
  template <typename T2>
  static constexpr auto get_binding2(const T2& t)
  {
    return get_binding<T2>(t);
  }
};

	template <typename T, typename l, char... str>
struct type_binding<String<str...>, T, Label<l>, type_location::remote_isValid> : public type_binding_super<String<str...>, bool, Label<l>>
{
  static_assert(is_handle<T>::value);
	static_assert(!std::is_same<T,bool>::value);

  constexpr type_binding() = default;
	using super = type_binding_super<String<str...>, bool, Label<l> >;
  using name = typename super::name;
  using label = typename super::label;

  using holder = remote_isValid_holder<T, str...>;
  using type = bool;

  template <typename Other>
  static constexpr mutils::mismatch get_binding(std::enable_if_t<!std::is_same<Other, name>::value, Other>)
  {
    return mutils::mismatch{};
  }
  template <typename>
  static constexpr type_binding get_binding(name)
  {
    return type_binding{};
  }
  template <typename T2>
  static constexpr auto get_binding2(const T2& t)
  {
    return get_binding<T2>(t);
  }
};

template <typename... T>
constexpr auto binding_to_holder(mutils::typeset<T...>)
{
  return mutils::typeset<typename T::holder...>{};
}

template <typename... T>
constexpr auto holder_to_value(mutils::typeset<T...>)
{
  return mutils::typeset<typename T::value...>{};
}
}
}

