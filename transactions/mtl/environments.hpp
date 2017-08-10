#pragma once

#include "Basics.hpp"
#include "CTString.hpp"
#include "Handle.hpp"
#include "TransactionContext.hpp"
#include "mtlutils.hpp"
#include "top.hpp"
#include <cassert>
#include <iostream>
#include <type_traits>
#include <vector>

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
  remote
};
template <typename Name, typename type, typename Label, type_location>
struct type_binding;

template <typename T, char... str>
struct value_holder
{

  using held_type = T;
  static constexpr int t_mem_length()
  {
    constexpr int div = (sizeof(unsigned long long) / sizeof(char));
    constexpr int adjusted_size = sizeof(T) / div;
    constexpr int ret = (adjusted_size * div < sizeof(T) ? 1 + adjusted_size : adjusted_size);
    return ret;
  }
  static constexpr int adjusted_T_size() { return t_mem_length() * sizeof(unsigned long long); }
  unsigned long long t_mem[t_mem_length()];
  T* zeroed_t()
  {
    bzero(t_mem, adjusted_T_size());
    return (T*)t_mem;
  }
  T& t{ *zeroed_t() };

  bool mem_uninitialized() const
  {
    for (const auto& word : t_mem)
      if (word != 0)
        return false;
    return true;
  }

  value_holder(T _t) { new (&t) T{ _t }; }
  value_holder() = default;
  value_holder(const value_holder& o) { new (&t) T{ o.t }; }
  value_holder& operator=(const value_holder& o)
  {
    if (mem_uninitialized())
      new (&t) T{ o.t };
    else
      t = o.t;
    return *this;
  }
  ~value_holder() { t.~T(); }

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
    assert(mem_uninitialized());
    new (&t) T{ t2 };
    return *this;
  }
  bool reset_index() { return true; }
  bool begin_phase() { return reset_index(); }

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
template <typename>
struct is_value_holder;
template <typename T, char... str>
struct is_value_holder<value_holder<T, str...>> : public std::true_type
{
};
template <typename T>
struct is_value_holder : public std::false_type
{
};

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
  bool begin_phase() { return reset_index(); }

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
    assert(curr_pos < (int)t.size() && curr_pos >= 0);
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
    assert(curr_pos < (int)t.size() && curr_pos >= 0);
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

template <typename>
struct is_type_holder;
template <typename T, char... str>
struct is_type_holder<type_holder<T, str...>> : public std::true_type
{
};
template <typename T>
struct is_type_holder : public std::false_type
{
};

template <typename, typename stored>
struct remote_map_holder
{
#ifndef NDEBUG
  bool is_initialized{ false };
  void initialize() { is_initialized = true; }
#endif

	template<typename U>
	void assign_to(U&& o) { this->operator=(std::forward<U>(o)); }

  std::map<Name, type_holder<stored>> super;
  bool reset_index()
  {
    assert(is_initialized);
    for (auto& holder : super) {
      holder.second.reset_index();
    }
    return true;
  }

  bool begin_phase()
  {
    assert(is_initialized);
    for (auto& holder : super) {
      holder.second.begin_phase();
    }
    return true;
  }
};

	template<typename T> struct is_remote_map_holder;
	template<typename T, typename Stored> struct is_remote_map_holder<remote_map_holder<T,Stored> > : public std::true_type{};
	
	template<typename... T>
	struct remote_map_aggregator : public virtual T... {
		static_assert((is_remote_map_holder<T>::value && ... && true),"Error: arguments must be remote_map_holders");
#ifndef NDEBUG
		void initialize(){
			((T::is_initialized = true), ...);
		}
#endif
		template<typename U>
		void assign_to(U&& u){
			(T::assign_to(std::forward<U>(u)),...);
		}
		
	};

template <typename T, typename stored, char... str>
struct remote_holder : public virtual remote_map_holder<T, stored>
{

  static_assert(is_handle<T>::value);

  // The idea is to protect against aliasing; we want
  // every binding site of the same handle to use the same type_holder.
  using super_t = type_holder<stored, str...>;
  using remote_map_holder<T, stored>::super;
  // we can re-bind this remote_holder, so
  // we really should be sure to keep a vector<handle>
  // around!
  std::vector<T> handle;
  int curr_pos{ -1 };

  using name = typename super_t::name;

  remote_holder() = default;
  remote_holder& operator=(const remote_holder& rh)
  {
    handle = rh.handle;
    curr_pos = rh.curr_pos;
    if (rh.super.size() > 0)
      remote_map_holder<T, stored>::operator=(rh);
    return *this;
  }
  remote_holder(const remote_holder& rh)
	  : remote_map_holder<T, stored>(rh)
    , handle(rh.handle)
    , curr_pos(rh.curr_pos)
  {
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

  // reset index + begin_phase are universal
  bool reset_index()
  {
    curr_pos = -1;
    return remote_map_holder<T, stored>::reset_index();
  }

  bool begin_phase() { return reset_index() && remote_map_holder<T, stored>::begin_phase(); }

  void increment()
  {
    if (curr_pos >= 0) {
      assert((int)handle.size() > curr_pos && curr_pos >= 0);
      this_super().increment();
    }
  }

  void increment_remote()
  {
    ++curr_pos;
    assert(curr_pos < ((int)handle.size()) && curr_pos >= 0);
  }

protected:
  void read_tracking_actions(_PhaseContext<typename T::label, true>& tc)
  {
#ifdef TRACK
    tc.trk_ctx.trk.checkForTombstones(tc, get_remote(tc).name());
    tc.trk_ctx.trk.record_timestamp(tc, get_remote(tc).timestamp());
#else
    (void)tc;
#endif
  }

  void read_tracking_actions(_PhaseContext<typename T::label, false>&) {}

  void bind_common(T& t)
  {
    handle.emplace_back(t);
    ++curr_pos;
    assert(curr_pos < ((int)handle.size()) && curr_pos >= 0);
  }

  auto& this_super()
  {
	  assert((remote_map_holder<T, stored>::is_initialized));
    return super[handle[curr_pos].name()];
  }

public:
  auto& bind(PhaseContext<typename T::label>& tc, T t)
  {
    bind_common(t);
    this_super().bind(tc, *handle[curr_pos].get(&tc));
    read_tracking_actions(tc);
    return *this;
  }

  template <typename... Args>
  remote_holder& push(PhaseContext<typename T::label>& tc, Args&&... args)
  {
    this_super().push(tc, std::forward<Args>(args)...);
    handle.back().put(&tc, this_super().t.back());
    return *this;
  }

  template <typename TransactionContext>
  auto get_remote(TransactionContext&)
  {
    assert(curr_pos < handle.size() && curr_pos >= 0);
    return handle.at(curr_pos);
  }

  template <typename TransactionContext>
  auto get(TransactionContext& tc)
  {
    assert(curr_pos < (int)handle.size() && curr_pos >= 0);
    // assert(super.count(handle.at(curr_pos)));
    return this_super().get(tc);
  }

  using value = remote_holder;
};

template <typename T, char... str>
struct remote_isValid_holder : public remote_holder<T, bool, str...>
{

  static_assert(is_handle<T>::value);

  // The idea is to protect against aliasing; we want
  // every binding site of the same handle to use the same type_holder.
  using super_t = remote_holder<T, bool, str...>;

  using name = typename super_t::name;

  remote_isValid_holder() = default;
  remote_isValid_holder& operator=(const remote_isValid_holder& rh)
  {
    super_t::operator=(rh);
    return *this;
  }

  remote_isValid_holder(const remote_isValid_holder& rh)
    : super_t(rh)
  {
  }

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

  // reset index + begin_phase are universal
  using super_t::reset_index;
  using super_t::begin_phase;
  using super_t::increment;
  using super_t::increment_remote;
  using super_t::this_super;

  auto& bind(PhaseContext<typename T::label>& tc, T t)
  {
    this->bind_common(t);
    bool is_valid = this->handle[this->curr_pos].isValid(&tc);
    this_super().bind(tc, is_valid);
    if (is_valid)
      this->read_tracking_actions(tc);
    return *this;
  }

  template <typename... Args>
  void push(PhaseContext<typename T::label>&, Args&&...)
  {
    static_assert((std::is_same<Args, void>::value && ... && true), "Error isValid is not assignable");
  }

  template <typename TransactionContext>
  auto get_remote(TransactionContext& tc)
  {
    return super_t::template get_remote<TransactionContext>(tc);
  }

  template <typename TransactionContext>
  auto get(TransactionContext& tc)
  {
    return super_t::template get<TransactionContext>(tc);
  }

  using value = remote_isValid_holder;
};

template <typename>
struct is_remote_holder;
template <typename T, typename v, char... str>
struct is_remote_holder<remote_holder<T, v, str...>> : public std::true_type
{
};
template <typename T, char... str>
struct is_remote_holder<remote_isValid_holder<T, str...>> : public std::true_type
{
};
template <typename T>
struct is_remote_holder : public std::false_type
{
};
template<typename T>
struct get_virtual_holders_str;
template<typename T, char... str> struct get_virtual_holders_str<remote_isValid_holder<T, str...> >{
	using type = remote_map_holder<T, bool>;
};

template<typename T, typename stored, char... str> struct get_virtual_holders_str<remote_holder<T, stored, str...> >{
	using type = remote_map_holder<T,stored>;
};

template<typename T> using get_virtual_holders = typename get_virtual_holders_str<T>::type;
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

template <typename t, char... str>
struct string_of<myria::mtl::remote_isValid_holder<t, str...>>
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
  static_assert(!std::is_same<T, typename T::type>::value);

  constexpr type_binding() = default;
  using super = type_binding_super<String<str...>, typename T::type, Label<l>>;
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

template <typename... T>
constexpr auto
binding_to_holder(mutils::typeset<T...>)
{
  return mutils::typeset<typename T::holder...>{};
}

template <typename... T>
constexpr auto
holder_to_value(mutils::typeset<T...>)
{
  return mutils::typeset<typename T::value...>{};
}
}
}
