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
#include "Massert.hpp"//*/
#include "SerializationMacros.hpp" 
#include "Transaction_macros.hpp"
#include "FreeExpr_macros.hpp"
#include "Operate_macros.hpp"

namespace myria{


	template<typename T, Level backbone, Level data, typename... DataSupportedOps>
	struct RemoteCons :
		public mutils::ByteRepresentable {
		using p = Handle<backbone, HandleAccess::all, RemoteCons,SupportedOperation<RegisteredOperations::Clone,SelfType,SelfType > >;
		using v = Handle<data, HandleAccess::all, T,DataSupportedOps...>;
		using newObj = const std::function<p (const typename p::stored_type&)>&;
	
		v val;
		p next;
	
		RemoteCons(const decltype(val) *val, const decltype(next) *next)
			:val(*val),next(*next){
			delete val; delete next;
		}

		RemoteCons(const decltype(val) &val, const decltype(next) &next)
			:val(val),next(next){}

	
        static p mke(newObj f, const std::function<v (const typename v::stored_type&)>& fv, const T &t ){
            return f(fv(t));
		}

        static p mke(newObj){ return p{};}

		template<typename Backbone, typename Data, typename... Args>
		static p build_list(tracker::Tracker& trk, mtl::TransactionContext *tc, Backbone& b, Data& d, const Args & ... args){
			auto tpl = std::make_tuple(args...);
			return fold(tpl,[&](const T &e, const auto & acc){
					RemoteCons initial{d.template newObject<HandleAccess::all,T>(trk,tc,e),acc};
					return b.template newObject<HandleAccess::all,RemoteCons>(trk,tc,initial);
				},p{});
		}

		bool operator==(const RemoteCons &wc) {
			std::stringstream ss1;
			std::stringstream ss2;
			ss1 << val << next;
			ss2 << wc.val << wc.next;
			return ss1.str() == ss2.str();
		}

		bool equals(const RemoteCons &wc) const {
			RemoteCons copy = *this;
			return copy == wc;
		}

		//DEFAULT_SERIALIZATION_SUPPORT(RemoteCons,val,next)
		  std::size_t to_bytes(char* ret) const { int sa = mutils::to_bytes(val,ret); return sa + mutils::to_bytes(next,ret + sa); } std::size_t bytes_size() const { return mutils::bytes_size(val) + mutils::bytes_size(next); } void post_object(const std::function<void (char const * const, std::size_t)>&f ) const { mutils::post_object(f,val); mutils::post_object(f,next); } static std::unique_ptr<RemoteCons> from_bytes(mutils::DeserializationManager* p, char const * v){ auto a2 = mutils::from_bytes<std::decay_t<decltype(val)> >(p,v); RemoteCons r{*a2,*(mutils::from_bytes<std::decay_t<decltype(next)> >(p,v + mutils::bytes_size(*a2)))}; return mutils::heap_copy(r); } void ensure_registered(mutils::DeserializationManager&){}
	};

	template<typename> struct is_remote_cons : std::false_type{};
	template<typename T2, Level backbone, Level data, typename... DataSupportedOps>
	struct is_remote_cons<RemoteCons<T2,backbone,data,DataSupportedOps...> > : std::true_type {};


	template<typename T2, Level backbone, Level data,typename... ops>
	std::ostream& operator<<(std::ostream &os, const RemoteCons<T2,backbone,data,ops...>& rc){
		return os << rc.val << " ++ " << rc.next;
	}
}
