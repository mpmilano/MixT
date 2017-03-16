#pragma once
#include "AST_split.hpp"
#include "split_printer.hpp"
#include "environments.hpp"

namespace myria {
namespace mtl {
namespace runnable_transaction {

template <typename... holders>
struct store : public holders...
{

  store() = default;
  store(const store&) = delete;

  template <typename val1, typename... values>
  store(const val1& val, const values&... vals)
    : store(vals...)
  {
    get(typename val1::name{}).bind(val.t);
  }

  template <char... str>
  auto& get(String<str...> name)
  {
    static_assert(mutils::contains_single_match<DECT(holders::template get_holder<String<str...>>(this, name))...>());
    using type_p = DECT(*mutils::find_match<DECT(holders::template get_holder<String<str...>>(this, name))...>());
    type_p ret = this;
    return *ret;
  }

  store& reset_indices()
  {
    bool b = (true && ... && holders::reset_index(this));
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

template <typename>
struct store_from_typeset_str;
template <typename... T>
struct store_from_typeset_str<mutils::typeset<T...>>
{
  using type = store<T...>;
};

template <typename T>
using store_from_typeset = typename store_from_typeset_str<T>::type;

template <typename l, typename AST, typename _provides, typename _inherits, typename reqs>
struct prephase
{
  constexpr prephase() = default;
  using level = l;
  using ast = AST;
  using requirements = reqs;
  using provides = _provides;
  using inherits = _inherits;
};

template <typename l, typename AST, typename reqs, typename provides, typename owns, typename passthrough>
struct phase;

template <typename l, typename AST, typename reqs, typename passthrough, typename owns, typename... _provides>
struct phase<l, AST, reqs, mutils::typeset<_provides...>, owns, passthrough>
{
  constexpr phase() = default;
  using ast = AST;
  using level = l;
  using requirements = reqs;
  using provides = mutils::typeset<_provides...>;
  using owned = owns;
};

template <typename... phases>
struct transaction
{
  constexpr transaction() = default;
  template <typename... t2>
  static constexpr auto append(transaction<t2...>)
  {
    return transaction<phases..., t2...>{};
  }

  using context = TransactionContext<typename phases::level...>;

  template <typename... env>
  using all_store =
    typename store_from_typeset<DECT(mutils::typelist_ns::combine(typename phases::provides{}...)
                                       .combine(mutils::typelist_ns::intersect(typename phases::requirements{}...))
                                       .combine(holder_to_value(mutils::typelist_ns::combine(typename phases::owned{}...))))>::template add<env...>;
};

template <typename...>
struct pretransaction;

template <>
struct pretransaction<>
{
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

  using next_transactions = typename pretransaction<prephases...>::processed;

  using processed = DECT(transaction<phase<typename p1::level, typename p1::ast, DECT(p1::requirements::subtract(typename p1::provides{})),
                                           DECT(p1::provides::intersect(next_requires{})), DECT(p1::provides::subtract(next_requires{})),
                                           DECT(p1::inherits::intersect(next_requires{}))>>::append(next_transactions{}));
};

template <typename l>
using AST = split_phase::AST<l>;

template <typename l, typename AST, typename provides, typename owns, typename passthrough, typename... reqs>
std::ostream& operator<<(std::ostream& o, phase<l, AST, mutils::typeset<reqs...>, provides, owns, passthrough>)
{
  using namespace mutils;
  auto print = [&](const auto& e) {
    print_varname(o, e);
    o << " ";
    return nullptr;
  };
  o << "Level " << l{} << ": requires ";
  auto ignore1 = { nullptr, nullptr, print(typename reqs::name{})... };
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
  return o;
}
}
}
}
