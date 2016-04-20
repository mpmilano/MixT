#pragma once
#include "Operation_macros.hpp"
namespace myria{

        //template<Level l, typename T> FINALIZE_OPERATION(Increment, 1, RemoteObject<l, T>* a);
template<Level l, typename T> auto Increment (mtl::TransactionContext* transaction_context, RemoteObject<l, T>* a) {
    return mutils::fold(mutils::fold_types<mutils::Pointerize,std::tuple<STORE_LIST>,std::tuple<> >{},
                        [&](const auto &arg, const auto &accum){
                            typedef std::decay_t<decltype(*arg)> Store;
                            typedef decltype(Store::Increment_impl(transaction_context, Store::tryCast(a))) ret_t;
                            ret_t def;
                            try {
                                auto ret = tuple_cons( Store::Increment_impl(transaction_context, Store::tryCast(a)) ,accum) ;
                                assert(std::get<0>(ret).built_well && "Did you actually implement this operation?"); return ret;
                            } catch (mtl::ClassCastException e){
                                return tuple_cons(def,accum);
                            } },
                        std::tuple<>());
    };

}
