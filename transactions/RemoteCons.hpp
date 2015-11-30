#pragma once
#include <sstream>
#include "IfBuilder.hpp"
#include "TransactionBuilder.hpp"
#include "Transaction.hpp"
#include "BaseCS.hpp"
#include "Operate.hpp"
#include "TempBuilder.hpp"
#include "FreeExpr.hpp"
#include "Print.hpp"
#include "Massert.hpp"
#include "SerializationMacros.hpp"
#include "Transaction_macros.hpp"
#include "FreeExpr_macros.hpp"
#include "Operate_macros.hpp"

namespace myria{


  template<typename T, Level backbone, Level data>
  struct RemoteCons :
    public mutils::ByteRepresentable {
    using p = Handle<backbone, HandleAccess::all, RemoteCons>;
    using v = Handle<data, HandleAccess::all, T>;
    using newObj = const std::function<p (const typename p::stored_type&)>&;
	
    v val;
    p next;
	
    RemoteCons(const decltype(val) *val, const decltype(next) *next)
      :val(*val),next(*next){
      delete val; delete next;
    }

    RemoteCons(const decltype(val) &val, const decltype(next) &next)
      :val(val),next(next){}

	
    static p mke(newObj){
      return p{};
    }

    template<typename Backbone, typename Data, typename... Args>
    static p build_list(Backbone& b, Data& d, const Args & ... args){
      auto tpl = std::make_tuple(args...);
      return fold(tpl,[&](const T &e, const auto & acc){
	  RemoteCons initial{d.template newObject<HandleAccess::all,T>(e),acc};
	  return b.template newObject<HandleAccess::all,RemoteCons>(initial);
	},p());
    }

    bool operator==(RemoteCons wc) {
      std::stringstream ss1;
      std::stringstream ss2;
      ss1 << val << next;
      ss2 << wc.val << wc.next;
      return ss1.str() == ss2.str();
    }

    bool equals(RemoteCons wc) const {
      RemoteCons copy = *this;
      return copy == wc;
    }

    DEFAULT_SERIALIZATION_SUPPORT(RemoteCons,val,next)	
  };


  template<typename T, Level backbone, Level data>
  std::ostream& operator<<(std::ostream &os, const RemoteCons<T,backbone,data>& rc){
    return os << rc.val << " ++ " << rc.next;
  }
}
