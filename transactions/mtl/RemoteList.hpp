#pragma once
#include "mutils-serialization/SerializationMacros.hpp"
#include "mutils-serialization/SerializationSupport.hpp"
#include <sstream>

namespace myria {
template <typename T, template <typename> class _Hndl>
struct RemoteList : public mutils::ByteRepresentable
{
  using Hndl = _Hndl<RemoteList>;

  T value;
  Hndl next;

  // DEFAULT_SERIALIZATION_SUPPORT(RemoteList, value, next);
  std::size_t to_bytes(char* ret) const
  {
    auto bytes_written1 = mutils::to_bytes(value, ret);
		auto bytes_written2 = mutils::to_bytes(next, ret + bytes_written1);
    return bytes_written1 + bytes_written2;
  }
  std::size_t bytes_size() const {
		auto r1 = mutils::bytes_size(value);
		auto r2 = mutils::bytes_size(next);
		return r1 + r2;
	}
  void post_object(const std::function<void(char const* const, std::size_t)>& func) const
  {
    mutils::post_object(func, value);
    mutils::post_object(func, next);
  }
	template<typename... ctxs>
  static std::unique_ptr<RemoteList> from_bytes(mutils::DeserializationManager<ctxs...>* dsm, char const* buf)
  {
    auto a_obj = mutils::from_bytes<std::decay_t<decltype(value)>>(dsm, buf);
    return std::make_unique<RemoteList>(*a_obj, *(mutils::from_bytes<std::decay_t<decltype(next)>>(dsm, buf + mutils::bytes_size(*a_obj))));
  }
	template<typename... ctxs>
  void ensure_registered(mutils::DeserializationManager<ctxs...>&){};

  RemoteList(const DECT(value) & v, const DECT(next) & n)
    : value(v)
    , next(n)
  {
  }

  // boilerplate; must find a way to eliminate it.
  bool is_struct{ true };
  auto& field(MUTILS_STRING(value)) { return value; }
  auto& field(MUTILS_STRING(next)) { return next; }
};
}

namespace mutils {
template <typename T, template <typename> class Hndl>
struct typename_str<myria::RemoteList<T, Hndl>>
{
  static std::string f()
  {
    std::stringstream ss;
    ss << "RemoteList<" << typename_str<T>::f() << ">";
    return ss.str();
  }
};
}
