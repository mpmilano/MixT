#pragma once
#include "AST_split.hpp"
#include "split_printer.hpp"
#include "environments.hpp"
#include "environments_serialization.hpp"
#include "mtlbasics.hpp"

namespace myria {
namespace mtl {
namespace runnable_transaction {

template <typename... holders>
struct store;

template <typename>
struct store_from_typeset_str;
template <typename... T>
struct store_from_typeset_str<mutils::typeset<T...>>
{
  using type = store<T...>;
};
template <typename T>
using store_from_typeset = typename store_from_typeset_str<T>::type;

template <typename, typename>
struct contains_name;
template <typename name, typename... members>
struct contains_name<name, mutils::typeset<members...>>
  : public std::integral_constant<bool, (std::is_same<typename members::name, name>::value || ... || false)>
{
};

template <typename, typename>
struct intersect_names_str;
template <typename rightset>
struct intersect_names_str<mutils::typeset<>, rightset>
{
  using type = mutils::typeset<>;
};
template <typename rightset, typename leftfirst, typename... leftsetmembers>
struct intersect_names_str<mutils::typeset<leftfirst, leftsetmembers...>, rightset>
{
  using type_start = std::conditional_t<contains_name<typename leftfirst::name, rightset>::value, mutils::typeset<leftfirst>, mutils::typeset<>>;
  using type = DECT(type_start::combine(typename intersect_names_str<mutils::typeset<leftsetmembers...>, rightset>::type{}));
};
template <typename left, typename right>
using intersect_names = typename intersect_names_str<left, right>::type;

template <typename... holders>
struct store : public holders...
{

  store() = default;
  store(store&&) = default;

  template <typename val1, typename... values>
  store(const val1& val, const values&... vals)
    : store(vals...)
  {
    PhaseContext<Label<top>> ctx;
    get(typename val1::name{}).bind(ctx, val.t);
  }

private:
  store(const store&) = default;
  store& operator=(store&&) = default;

public:
  store clone() { return *this; }
  store& take(store s) { return this->operator=(std::move(s)); }

  template <char... str>
  auto& get(String<str...> name)
  {
    static_assert(mutils::contains_single_match<DECT(holders::template get_holder<String<str...>>(this, name))...>());
    using type_p = DECT(*mutils::find_match<DECT(holders::template get_holder<String<str...>>(this, name))...>());
    type_p ret = this;
    return *ret;
  }

  store& begin_phase()
  {
    bool b = (true && ... && holders::begin_phase());
    (void)b;
    return *this;
  }

  store& rollback_phase()
  {
    bool b = (true && ... && holders::rollback_phase());
    (void)b;
    return *this;
  }

  static constexpr store* add_f()
  {
    constexpr store* ret{ nullptr };
    return ret;
  }
  template <typename T, typename... U>
  static constexpr auto* add_f(T*, U*...)
  {
    return std::conditional_t<mutils::contained<typename T::name, typename holders::name...>(), store, store<holders..., T>>::add_f(((U*)nullptr)...);
  }

  template <typename... T>
  using add = DECT(*add_f(((T*)nullptr)...));

  using holder_count = std::integral_constant<std::size_t, sizeof...(holders)>;

  template <typename Name>
  using find_holder_by_name =
    DECT(*mutils::find_match<std::conditional_t<std::is_same<typename Name::name, typename holders::name>::value, holders, mutils::mismatch>...>());

  template <typename... holder_names>
  using restrict_to_holders = store_from_typeset<intersect_names<mutils::typeset<holders...>, mutils::typeset<holder_names...>>>;

  template <typename phase>
  using restrict_to_phase =
    store_from_typeset<intersect_names<mutils::typeset<holders...>, DECT(phase::requirements::combine(phase::provides::combine(typename phase::owned{})))>>;
};

template <typename>
struct is_store;
template <typename... holders>
struct is_store<store<holders...>> : public std::true_type
{
};
template <typename>
struct is_store : public std::false_type
{
};

template <typename l, typename _returns, typename AST, typename _provides, typename _inherits, typename reqs>
struct prephase
{
  constexpr prephase() = default;
  using level = l;
  using returns = _returns;
  using ast = AST;
  using requirements = reqs;
  using provides = _provides;
  using inherits = _inherits;
};

template <txnID_t id, typename l, typename _returns, typename AST, typename reqs, typename provides, typename owns, typename passthrough>
struct phase;

template <txnID_t id, typename l, typename _returns, typename AST, typename reqs, typename passthrough, typename owns, typename... _provides>
struct phase<id, l, _returns, AST, reqs, mutils::typeset<_provides...>, owns, passthrough>
{
  constexpr phase() = default;
  using ast = AST;
  using label = l;
  using requirements = reqs;
  using returns = _returns;
  using provides = mutils::typeset<_provides...>;
  using owned = owns;
  using txnID = std::integral_constant<txnID_t, id>;

  template <typename label>
  using has_label = std::conditional_t<std::is_same<l, label>::value, phase, mutils::mismatch>;
};
template <typename... phases>
struct transaction;

template <>
struct transaction<>
{
  constexpr transaction() = default;
  template <typename... t2>
  static constexpr auto append(transaction<t2...> a)
  {
    return a;
  }

  template <typename... env>
  using all_store = store<env...>;
};

template <typename p1, typename... phases>
struct transaction<p1, phases...>
{
  constexpr transaction() = default;
  template <typename... t2>
  static constexpr auto append(transaction<t2...>)
  {
    return transaction<p1, phases..., t2...>{};
  }

  using number_remote_phases = std::integral_constant<std::size_t, (phases::label::run_remotely::value + ... + 0)>;

  template <typename... env>
  using all_store = typename store_from_typeset<DECT(
    mutils::typelist_ns::combine(typename p1::provides{}, typename phases::provides{}...)
      .combine(mutils::typelist_ns::intersect(typename p1::requirements{}, typename phases::requirements{}...))
      .combine(holder_to_value(mutils::typelist_ns::combine(typename p1::owned{}, typename phases::owned{}...))))>::template add<env...>;

  template <typename label>
  using find_phase = DECT(*mutils::find_match<typename p1::template has_label<label>, typename phases::template has_label<label>...>());
};

template <typename...>
struct pretransaction;

template <>
struct pretransaction<>
{
  template <txnID_t>
  using processed = transaction<>;
};

template <typename p1, typename... prephases>
struct pretransaction<p1, prephases...>
{
  constexpr pretransaction() = default;
  template <typename... t2>
  static constexpr auto append(pretransaction<t2...>)
  {
    return pretransaction<p1, prephases..., t2...>{};
  }

  using next_requires = DECT(p1::requirements::combine(typename prephases::requirements{}...));

  template <txnID_t id>
  using next_transactions = typename pretransaction<prephases...>::template processed<id + 1>;

  template <txnID_t id>
  using processed =
    DECT(transaction<phase<id, typename p1::level, typename p1::returns, typename p1::ast, DECT(p1::requirements::subtract(typename p1::provides{})),
                           DECT(p1::provides::intersect(next_requires{})), DECT(p1::provides::subtract(next_requires{})),
                           DECT(p1::inherits::intersect(next_requires{}))>>::append(next_transactions<id>{}));
};

template <typename l>
using AST = split_phase::AST<l>;

template <txnID_t id, typename l, typename returns, typename AST, typename provides, typename owns, typename passthrough, typename... reqs>
std::ostream& operator<<(std::ostream& o, phase<id, l, returns, AST, mutils::typeset<reqs...>, provides, owns, passthrough>)
{
  using namespace mutils;
  auto print = [&](const auto& e) {
    print_varname(o, e);
    o << " ";
    return nullptr;
  };
  o << "Phase number: " << id << " returning " << type_name<returns>() << std::endl;
  o << "Level " << l{} << ": requires ";
  auto ignore1 = { nullptr, nullptr, print(typename reqs::name{})... };
  (void)ignore1;
  o << std::endl;
  o << "Level " << l{} << ": provides " << provides{} << std::endl;
  o << "Level " << l{} << ": owns " << owns{} << std::endl;
  split_phase::template print_ast<l>(o, AST{}, "");
  return o << std::endl;
}

template <typename... phase>
std::ostream& operator<<(std::ostream& o, transaction<phase...>)
{
  auto print = [&](const auto& e) {
    o << e;
    return nullptr;
  };
  auto ignore = { nullptr, nullptr, print(phase{})... };
  (void)ignore;
  return o;
}
}
}
}
