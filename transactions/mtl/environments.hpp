#pragma once

#include "Basics.hpp"
#include "mutils/CTString.hpp"
#include "Handle.hpp"
#include "mtl/TransactionContext.hpp"
#include "mtl/mtlutils.hpp"
#include "mtl/top.hpp"
#include "mtl/pre_endorse.hpp"
#include "mtl/type_environment.hpp"
#include <cassert>
#include <iostream>
#include <type_traits>
#include <vector>
#include <array>

namespace myria {

namespace mtl {

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
	std::array<unsigned long long, t_mem_length()> t_mem{{}};
	T& zeroed_t_mem(){
		bzero(t_mem.data(),sizeof(t_mem));
		return *(T*) t_mem.data();
	}
	static_assert(sizeof(t_mem) >= sizeof(T));
	T& t =  zeroed_t_mem();

	bool mem_uninitialized{true};

	value_holder(T _t):mem_uninitialized{false} { new (&t) T{ _t }; }
	value_holder() = default;
	value_holder(const value_holder& o):mem_uninitialized(o.mem_uninitialized)
		{
			if (!o.mem_uninitialized) new (&t) T{ o.t };
		}
  value_holder& operator=(const value_holder& o)
  {
	  if (mem_uninitialized && !o.mem_uninitialized){
		  mem_uninitialized = false;
		  new (&t) T{ o.t };
	  }
	  else if (!mem_uninitialized && !o.mem_uninitialized)
		  t = o.t;
	  else {
/*receiving uninitialized*/
		  zeroed_t_mem();
		  mem_uninitialized = true;
	  }
	  return *this;
  }

  value_holder& operator=(const T& newt){
      if (mem_uninitialized) {
          mem_uninitialized = false;
          new (&t) T{newt};
      }
      else t = newt;
      return *this;
  }

  ~value_holder() {if (!mem_uninitialized) t.~T(); }

  using type = T;
  using name = mutils::String<str...>;
  template <typename TransactionContext>
  T& get(value_holder& whendebug(_this), TransactionContext&)
  {
	  assert(&_this == this);
	  assert(!mem_uninitialized);
    return t;
  }

  template <typename TransactionContext>
  value_holder& push(value_holder& whendebug(_this), TransactionContext&, const T& t2)
  {
	  assert(&_this == this);
	  assert(!mem_uninitialized);
    t = t2;
    return *this;
  }
  template <typename TransactionContext>
  value_holder& bind(value_holder& whendebug(_this), TransactionContext& ctx, const T& t2)
  {
      (void)ctx;
	  assert(&_this == this);

	  //we might be in a while-loop, at which point
	  //we want to destroy + replace the value here.
	  if (!mem_uninitialized) t.~T();
    new (&t) T{ t2 };
	mem_uninitialized = false;
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

template <>
struct value_holder<bool,'t','r','u','e'>
{

  template <typename ignore, typename TransactionContext>
  static const bool& get(ignore& , TransactionContext&)
  {
		static const bool ret{true};
    return ret;
  }
};
	
template <>
struct value_holder<bool,'f','a','l','s','e'>
{

  template <typename ignore, typename TransactionContext>
  static const bool& get(ignore& , TransactionContext&)
  {
		static const bool ret{false};
    return ret;
  }
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
struct value_with_stringname_str<T, mutils::String<str...>>
{
  using type = value<T, str...>;
};

template <typename T, typename N>
using value_with_stringname = typename value_with_stringname_str<T, N>::type;

template <typename T, char... str>
struct type_holder
{

  using held_type = T;
  using name = mutils::String<str...>;

  std::vector<T> t;
  int curr_pos{ -1 };
  bool bound = false;

  type_holder() = default;

	type_holder(value<T, str...> v) { bind(*this,v.t); }

  bool reset_index()
  {
    curr_pos = -1;
    return true;
  }
  bool begin_phase() { return reset_index(); }

  template <typename TranCtx, typename... Args>
  type_holder& push(type_holder& whendebug(_this), TranCtx&, Args&&... args)
  {
	  assert(&_this == this);
    t.emplace_back(std::forward<Args>(args)...);
    ++curr_pos;
    return *this;
  }

  template <typename TransactionContext>
  type_holder& bind(type_holder& whendebug(_this), TransactionContext&, T _t)
  {
	  assert(&_this == this);
    bound = true;
    t.emplace_back(_t);
    ++curr_pos;
    assert(curr_pos < (int)t.size() && curr_pos >= 0);
    return *this;
  }

  type_holder& increment(type_holder& _this)
  {
	  assert(&_this == this);
    ++_this.curr_pos;
    return *this;
  }
  template <typename TranCtx>
  T get(type_holder& whendebug(_this), TranCtx&)
  {
	  assert(&_this == this);
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

template <typename T>
struct remote_map_holder;
	
	template <typename l, typename U, typename... SupportedOps>
	struct remote_map_holder<Handle<l,U,SupportedOps...> >
{

	using Handle_t = Handle<l,U,SupportedOps...>;
	using stored = U;
#ifndef NDEBUG
  bool is_initialized{ false };
  void initialize() { is_initialized = true; }
	remote_map_holder(const remote_map_holder&) = default;
	remote_map_holder(remote_map_holder&&) = default;
	remote_map_holder& operator=(const remote_map_holder&) = default;
	remote_map_holder(bool):is_initialized(true){}
#endif

	template<typename T>
	void assign_to(T&& o) { this->operator=(std::forward<T>(o)); }

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
	void increment_matching(const Handle_t &hndl){
		auto _this = super[hndl.name()];
		_this.increment(_this);
	}
};

	template<typename T> struct is_remote_map_holder;
	template<typename T> struct is_remote_map_holder<remote_map_holder<T> > : public std::true_type{};
	
	template<typename... T>
	struct remote_map_aggregator : public T... {
		static_assert((is_remote_map_holder<T>::value && ... && true),"Error: arguments must be remote_map_holders");
#ifndef NDEBUG
		void initialize(){
			((T::is_initialized = true), ...);
		}
		remote_map_aggregator(bool b):T(b)...{}
		remote_map_aggregator(const remote_map_aggregator&) = default;
		remote_map_aggregator(remote_map_aggregator&&) = default;
		remote_map_aggregator& operator=(const remote_map_aggregator&) = default;
		remote_map_aggregator& operator=(remote_map_aggregator&&) = default;

		template<typename K> struct get_specific_holder_str {
			template<typename> struct handle_type_str;
			template<typename l, typename U, typename... ops> struct handle_type_str<remote_map_holder<Handle<l,U,ops...> > >{
				using type = U;
			};
			template<typename U> using handle_type = typename handle_type_str<U>::type;
			template<typename U> using matches = std::is_same<K,handle_type<U> >;
			using type_list = DECT(mutils::typelist<T...>::template filter<matches>());

			template<typename fst, typename... rst>
			static auto run(const mutils::typelist<fst,rst...>&){
				struct inner{
					static auto* run(fst* _this){ return _this;}
				};
				return inner{};
			}			

			static auto run(const mutils::typelist<>&){
				struct inner{
					static std::nullptr_t run(...){ return nullptr;}
				};
				return inner{};
			}
			
			template<typename U>
			static auto& run(U& _this){
				return *get_specific_holder_str::run(type_list{}).run(&_this);
			}
		};

		template<typename K>
		auto& get_specific_holder(){
			return get_specific_holder_str<K>::run(*this);
		}
#endif
		template<typename U>
		void assign_to(U&& u){
			(T::assign_to(std::forward<U>(u)),...);
		}

		auto begin_phase(){
			return (T::begin_phase() && ... && true);
		}

		auto rollback_phase(){
			return (T::rollback_phase() && ... && true);
		}

		remote_map_aggregator& as_virtual_holder(){
			return *this;
		}
		
	};

template <typename T, char... str>
struct remote_holder 
{

  static_assert(is_handle<T>::value);

  // The idea is to protect against aliasing; we want
  // every binding site of the same handle to use the same type_holder.
	using stored = typename remote_map_holder<T>::stored;
  // we can re-bind this remote_holder, so
  // we really should be sure to keep a vector<handle>
  // around!
  std::vector<T> handle;
  int curr_pos{ -1 };

  using name = typename type_holder<stored, str...>::name;

  remote_holder() = default;
	remote_holder& operator=(const remote_holder& rh) = default;
	remote_holder(const remote_holder& rh) = default;

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

  bool begin_phase() {
	  curr_pos = -1;
	  return true;
  }

	template<typename Store>
  static void increment(Store &s)
  {
	  remote_holder &_this = s;
    if (_this.curr_pos >= 0) {
      assert((int)_this.handle.size() > _this.curr_pos && _this.curr_pos >= 0);
      auto &_this_super = this_super(s);
    _this_super.increment(_this_super);
    }
  }

	template<typename Store>
  static void increment_remote(Store &s)
  {
	  remote_holder &_this = s;
    ++_this.curr_pos;
    assert(_this.curr_pos < ((int)_this.handle.size()) && _this.curr_pos >= 0);
	increment(s);
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

	template<typename Store>
  static type_holder<stored>& this_super(Store &s)
  {
	  remote_map_holder<T> &_super = s.as_virtual_holder();
	  remote_holder &_this = s;
	  assert(_super.is_initialized);
    return _super.super[_this.handle[_this.curr_pos].name()];
  }

	//Note: we need to do replacements before run_phase,
	//because otherwise we get mismatched phase contexts.
	//if you're here due to an error matching PhaseContext,
	//that's probably why.
public:
	template<typename Store>
	static auto& bind(Store &s, PhaseContext<typename T::label>& tc, T t)
  {
	  constexpr String<str...> name;
	  (void)name;
	  remote_holder& _this = s;
    _this.bind_common(t);
	auto &_this_super = this_super(s);
	auto &bind_target = *_this.handle[_this.curr_pos].get(&tc);
    _this_super.bind(_this_super,tc, bind_target);
    _this.read_tracking_actions(tc);
    return _this;
  }

	template <typename Store, typename... Args>
	static remote_holder& push(Store &s, PhaseContext<typename T::label>& tc, Args&&... args)
  {
	  remote_holder &_this = s;
	  auto &_this_super = this_super(s);
    _this_super.push(_this_super,tc, std::forward<Args>(args)...);
    _this.handle.back().put(&tc,_this_super.t.back());
    return _this;
  }

  template <typename TransactionContext>
  auto get_remote(TransactionContext&)
  {
    assert(curr_pos < (long long)handle.size() && curr_pos >= 0);
    return handle.at(curr_pos);
  }

	template <typename Store, typename TransactionContext>
	static auto get(Store &s, TransactionContext& tc)
  {
	  whendebug(remote_holder &_this = s);
    assert(_this.curr_pos < (int)_this.handle.size() && _this.curr_pos >= 0);
    // assert(super.count(handle.at(curr_pos)));
    auto &_this_super = this_super(s);
    return _this_super.get(_this_super,tc);
  }

  using value = remote_holder;
};

template <typename>
struct is_remote_holder;
template <typename T, char... str>
struct is_remote_holder<remote_holder<T, str...>> : public std::true_type
{
};
	template <typename T, char... str>
	struct is_remote_holder<value_holder<T,str...> > : public std::false_type
{
};
	template <typename T, char... str>
	struct is_remote_holder<type_holder<T,str...> > : public std::false_type
{
};
template<typename T>
struct get_virtual_holders_str;

template<typename T, char... str> struct get_virtual_holders_str<remote_holder<T, str...> >{
	using type = remote_map_holder<T>;
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

template <typename t, char... str>
struct string_of<myria::mtl::remote_holder<t, str...>>
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

template <typename, typename>
struct type_binding_super;

template <typename T, char... str>
struct type_binding_super<String<str...>, T>
{

  using name = String<str...>;

  constexpr type_binding_super() = default;
};

template <typename T, typename l, char... str>
struct type_binding<String<str...>, T, Label<l>, type_location::local> : public type_binding_super<String<str...>, T>
{
  using holder = type_holder<T, str...>;

  constexpr type_binding() = default;
  using super = type_binding_super<String<str...>, T>;
  using name = typename super::name;
  using label = Label<l>;
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

	template<typename L, typename R> struct decide_type_label_str;
	template<typename L, typename R>
	struct decide_type_label_str<Label<PreEndorse<L> >, R>{
		using type = resolved_label_min<Label<PreEndorse_notop<L> >, Label< PreEndorse_notop<R> > >;
	};
	template<typename L, typename R>
	struct decide_type_label_str<L,Label<PreEndorse<R> > >{
		using type = resolved_label_min<Label<PreEndorse_notop<L> >, Label<PreEndorse_notop<R> > >;
	};
	template<typename L, typename R> struct decide_type_label_str{
		using type = resolved_label_min<L,R>;
	};
	
	template<typename L, typename R>
	using decide_type_label = typename decide_type_label_str<L,R>::type;

template <typename T, typename l, char... str>
//note; if Label<l> is a pre-endorse label, then we should also have the handle bind at pre-endorse.
struct type_binding<String<str...>, T, Label<l>, type_location::remote> : public type_binding_super<String<str...>, typename T::type>
{
  static_assert(is_handle<T>::value);
  static_assert(!std::is_same<T, typename T::type>::value);

  constexpr type_binding() = default;
  using super = type_binding_super<String<str...>, typename T::type>;
  using name = typename super::name;
  using label = decide_type_label<Label<l>, typename T::label>;

  using holder = remote_holder<T, str...>;
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
